#include <deque>
#include <fstream>

#include "estimate_psf_form/Fitter.h"

#include <Eigen/StdVector>
#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/variant/apply_visitor.hpp>

#include "constant_background/model.hpp"
#include "engine/InputTraits.h"
#include "engine/JobInfo.h"
#include "estimate_psf_form/Config.h"
#include "estimate_psf_form/LocalizationValueFinder.h"
#include "estimate_psf_form/VariableReduction.h"
#include "fit_window/chunkify.hpp"
#include "fit_window/FitWindowCutter.h"
#include "fit_window/Optics.h"
#include "gaussian_psf/DepthInfo3D.h"
#include "gaussian_psf/No3D.h"
#include "guf/FitFunctionFactoryImplementation.hpp"
#include "guf/Spot.h"
#include "Localization.h"
#include "nonlinfit/levmar/Fitter.h"
#include "nonlinfit/plane/Joint.h"
#include "nonlinfit/sum/AbstractFunction.h"
#include "nonlinfit/terminators/StepLimit.h"
#include "threed_info/No3D.h"
#include "threed_info/Spline3D.h"

#include "debug.h"

namespace dStorm {
namespace estimate_psf_form {

using namespace nonlinfit;

/** Specialization of a FittingVariant for a concrete PSF model and
 *  parameter assignment.
 *
 *  \tparam Lambda A lambda for a subclass of PSF::BaseExpression to fit to the data
 */
template <typename Lambda>
class Fitter
: public FittingVariant
{
    typedef sum::AbstractFunction CombinedFunction;

    typedef boost::mpl::vector<nonlinfit::plane::xs_joint<double, 8>::type> DataTags;
    typedef guf::FitFunctionFactoryImplementation<Lambda, DataTags> FitFunctionFactory;

    const Config config;
    fit_window::FitWindowCutter window_cutter;
    std::vector<std::unique_ptr<FitFunctionFactory>> models;
    std::vector<std::unique_ptr<guf::FitFunction>> functions;
    const dStorm::engine::InputTraits& traits;
    VariableReduction<typename boost::mpl::joint_view<
        typename Lambda::Variables,
        constant_background::Expression::Variables>::type> table;

    /** Get one of the model instances matching the given fluorophore type and layer. */
    const Lambda& result( int fluorophore = -1, int layer = 0 ) {
        const int i = ( fluorophore == -1 ) ? layer : table.find_plane(layer, fluorophore);
        assert( i >= 0 && i < int(models.size()) );
        return models[i]->get_gaussian();
    }

    boost::shared_ptr<const threed_info::DepthInfo> get_3d( const gaussian_psf::DepthInfo3D& s, int plane, Direction dir ) {
        return traits.optics(plane).depth_info(dir);
    }

    boost::shared_ptr<const threed_info::DepthInfo> get_3d( const gaussian_psf::No3D& m, int plane, Direction dir ) {
        boost::shared_ptr<threed_info::No3D> rv( new threed_info::No3D() );
        rv->sigma = threed_info::Sigma( m.get< gaussian_psf::BestSigma >(dir) );
        return rv;
    }

  public:
    /** \see FittingVariant::create(). */
    Fitter( const Config& config, const input::Traits< engine::ImageStack >& traits, int images );
    /** \see FittingVariant::add_image(). */
    bool add_image( const engine::ImageStack& image, const Localization& position, int fluorophore );
    /** \see FittingVariant::fit(). */
    void fit( input::Traits< engine::ImageStack >& new_traits, simparm::ProgressEntry& progress );
    double collection_state() const { return table.collection_state(); }
};

template <class Lambda>
Fitter<Lambda>::Fitter( const Config& config, const input::Traits< engine::ImageStack >& traits, int images )
: config(config), window_cutter(config.fit_window_config, traits, std::set<int>(), 0),
  traits(traits), table( config, traits.fluorophores.size(), traits.plane_count() > 1, images * traits.plane_count() )
{
    DEBUG("Creating form fitter");
}

template <class Lambda>
bool Fitter<Lambda>::
add_image( const engine::ImageStack& image, const Localization& position, int fluorophore ) 
{
    guf::Config guf_config;
    guf_config.allow_disjoint = false;
    guf_config.laempi_fit = config.laempi_fit();
    guf_config.disjoint_amplitudes = config.disjoint_amplitudes();
    guf_config.double_computation = true;
    guf_config.negligible_x_step = 0.1f * boost::units::si::nanometre;
    guf_config.relative_epsilon = 1E-4;

    guf::Spot spot_position;
    spot_position.x() = position.position_x() / (1E-6 * si::meter);
    spot_position.y() = position.position_y() / (1E-6 * si::meter);
    fit_window::PlaneStack stack = window_cutter.cut_region_of_interest(image, spot_position);
    for (int i = 0; i < image.plane_count(); ++i) {
        DEBUG("Adding layer " << i << " of " << image.plane_count() << " to model with " << models.size()
                << " evaluators");
        if ( ! table.needs_more_planes() ) return true;

        std::unique_ptr<FitFunctionFactory> new_model( new FitFunctionFactory(guf_config, 1, true) );

        LocalizationValueFinder iv(fluorophore, traits.optics(i), position, i);
        iv.find_values( new_model->get_gaussian() );
        iv.find_values( new_model->get_background() );
        new_model->get_gaussian().allow_leaving_ROI( true );

        functions.push_back(new_model->create_function(stack[i], config.mle()));
        models.push_back( std::move(new_model) );
    }
    table.add_planes( image.plane_count(), fluorophore );
    return ! table.needs_more_planes();
}

template <class Lambda>
void Fitter<Lambda>::fit( input::Traits< engine::ImageStack >& new_traits, simparm::ProgressEntry& progress ) 
{
    progress.indeterminate = true;
    progress.setValue( 0.5 );

    CombinedFunction combiner( table.get_reduction_matrix() );
    for (size_t i = 0; i < functions.size(); ++i) {
        combiner.set_fitter(i, *functions[i]->abstract_function());
    }

    nonlinfit::levmar::Fitter fitter = nonlinfit::levmar::Config();
    nonlinfit::terminators::StepLimit step_limit(300);
    fitter.fit( combiner, step_limit );

    progress.indeterminate = false;
    progress.setValue( 1 );

    for (int j = 0; j < traits.plane_count(); ++j) {
        for (Direction dir = Direction_First; dir != Direction_2D; ++dir)
            new_traits.optics(j).set_depth_info( dir, get_3d( result(0,j), j, dir ) );
    }
    for (size_t i = 0; i < traits.fluorophores.size(); ++i) {
        if ( ! table.has_fluorophore( i ) ) {
            std::cerr << "Have seen no examples of fluorophore " << i << " and left its parameters unchanged." << std::endl;
            continue;
        }
        float target_transmission = 0, total_transmission = 0; 
        for (int j = 0; j < traits.plane_count(); ++j) {
            total_transmission += result(i,j)( gaussian_psf::Prefactor() );
            target_transmission += traits.optics(j).transmission_coefficient(i);
        }
        for (int j = 0; j < traits.plane_count(); ++j) {
            new_traits.optics(j).set_fluorophore_transmission_coefficient(i, 
                result(i,j)( gaussian_psf::Prefactor() )
                    * target_transmission / total_transmission );
        }
    }
}

template <typename Lambda, typename DepthInfo>
std::auto_ptr<FittingVariant>
create2( const Config& config, const input::Traits< engine::ImageStack >& traits, int images ) {
    for ( input::Traits< engine::ImageStack >::const_iterator i = traits.begin(); i != traits.end(); ++i )
        for (Direction dir = Direction_First; dir != Direction_2D; ++dir)
            if ( dynamic_cast< const DepthInfo* >( i->optics.depth_info(dir).get() ) == NULL ) {
                throw std::runtime_error("3D PSF models need to be consistent for form fitting");
            }
    return std::auto_ptr<FittingVariant>( new Fitter< Lambda > ( config, traits, images ) );
}

std::auto_ptr<FittingVariant>
FittingVariant::create( const Config& config, const input::Traits< engine::ImageStack >& traits, int images )
{
    const threed_info::DepthInfo* d = traits.optics(0).depth_info(Direction_X).get();
    if ( dynamic_cast< const threed_info::No3D* >(d) )
        return create2<gaussian_psf::No3D,threed_info::No3D>( config, traits, images );
    else if ( dynamic_cast< const threed_info::Spline3D* >(d) )
        return create2<gaussian_psf::DepthInfo3D,threed_info::Spline3D>( config, traits, images );
    else
        throw std::logic_error("Missing 3D model in form fitter");
}

}
}
