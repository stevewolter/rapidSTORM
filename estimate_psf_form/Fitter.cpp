#include "estimate_psf_form/decl.h"
#include <Eigen/StdVector>
#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/variant/apply_visitor.hpp>
#include "estimate_psf_form/Fitter.h"
#include "estimate_psf_form/Config.h"
#include "estimate_psf_form/VariableReduction.h"
#include "image/slice.h"
#include "Localization.h"
#include "engine/JobInfo.h"
#include "guf/Spot.h"
#include "fit_window/chunkify.hpp"
#include "nonlinfit/plane/Distance.hpp"
#include "nonlinfit/plane/JointData.h"
#include "nonlinfit/plane/Joint.h"
#include "nonlinfit/plane/JointTermImplementation.h"
#include "nonlinfit/Bind.h"
#include "nonlinfit/sum/AbstractFunction.h"
#include "gaussian_psf/parameters.h"
#include "gaussian_psf/DepthInfo3D.h"
#include "gaussian_psf/No3D.h"
#include "gaussian_psf/fixed_form.h"
#include "gaussian_psf/JointEvaluator.h"
#include "nonlinfit/levmar/Fitter.h"
#define BOOST_DETAIL_CONTAINER_FWD_HPP
#include <boost/lambda/lambda.hpp>
#include <boost/variant/get.hpp>
#include "engine/InputTraits.h"
#include <fstream>

#include "fit_window/chunkify.h"
#include "fit_window/Optics.h"
#include "fit_window/FitWindowCutter.h"

#include <nonlinfit/terminators/StepLimit.h>
#include "threed_info/No3D.h"
#include "threed_info/Spline3D.h"

#include "estimate_psf_form/LocalizationValueFinder.h"
#include "constant_background/model.hpp"

#include "debug.h"

namespace dStorm {
namespace estimate_psf_form {

using namespace nonlinfit;

/** Specialization of a FittingVariant for a concrete PSF model and
 *  parameter assignment.
 *
 *  \tparam Metric Metric tag to use
 *  \tparam Lambda A lambda for a subclass of PSF::BaseExpression to fit to the data
 */
template <class Metric, typename Lambda>
class Fitter
: public FittingVariant
{
    struct less_amplitude;

    typedef plane::xs_joint<double,2>::type DataTag;
    typedef sum::AbstractFunction CombinedFunction;

    class PlaneFunction : public nonlinfit::AbstractFunction<double> {
        Lambda gaussian;
        nonlinfit::plane::JointTermImplementation<Lambda, DataTag> gaussian_term;
        constant_background::Expression background;
        nonlinfit::plane::JointTermImplementation<constant_background::Expression, DataTag> background_term;
        nonlinfit::plane::Distance< DataTag, Metric > function;
        nonlinfit::plane::JointData<double, 2> xs;

      public:
        PlaneFunction(const fit_window::Plane& plane)
            : gaussian_term(gaussian), background_term(background),
              function(std::vector<nonlinfit::plane::Term<DataTag>*>{&gaussian_term, &background_term}) {
            gaussian.set_relative_epsilon(1E-4);
            gaussian.set_negligible_step_length(1E-4);
            background.set_relative_epsilon(1E-4);
            chunkify(plane, xs);
            function.set_data(xs);
        }

        Lambda& get_gaussian() { return gaussian; }
        constant_background::Expression& get_background() { return background; }

        int variable_count() const OVERRIDE { return function.variable_count(); }
        bool evaluate( Derivatives& p ) OVERRIDE { return function.evaluate(p); }
        void get_position( Position& p ) const OVERRIDE { function.get_position(p); }
        void set_position( const Position& p ) OVERRIDE { function.set_position(p); }
        bool step_is_negligible( const Position& old_position, const Position& new_position ) const OVERRIDE {
            return function.step_is_negligible(old_position, new_position);
        }
    };

    /** Optics indexed by input layer. */
    fit_window::FitWindowCutter window_cutter;
    typedef boost::ptr_vector< PlaneFunction > Evaluators;
    Evaluators evaluators;
    const dStorm::engine::InputTraits& traits;
    VariableReduction<typename boost::mpl::joint_view<
        typename Lambda::Variables,
        constant_background::Expression::Variables>::type> table;

    /** Get one of the model instances matching the given fluorophore type and layer. */
    const Lambda& result( int fluorophore = -1, int layer = 0 ) {
        const int i = ( fluorophore == -1 ) ? layer : table.find_plane(layer, fluorophore);
        assert( i >= 0 && i < int(evaluators.size()) );
        return evaluators[i].get_gaussian();
    }

    boost::shared_ptr<const threed_info::DepthInfo> get_3d( const gaussian_psf::DepthInfo3D& s, int plane, Direction dir ) {
        return traits.optics(plane).depth_info(dir);
    }

    boost::shared_ptr<const threed_info::DepthInfo> get_3d( const gaussian_psf::No3D& m, int plane, Direction dir ) {
        boost::shared_ptr<threed_info::No3D> rv( new threed_info::No3D() );
        rv->sigma = threed_info::Sigma( m.get< gaussian_psf::BestSigma >(dir) );
        return rv;
    }

    void apply_z_calibration();

  public:
    /** \see FittingVariant::create(). */
    Fitter( const Config& config, const input::Traits< engine::ImageStack >& traits, int images );
    /** \see FittingVariant::add_image(). */
    bool add_image( const engine::ImageStack& image, const Localization& position, int fluorophore );
    /** \see FittingVariant::fit(). */
    void fit( input::Traits< engine::ImageStack >& new_traits, simparm::ProgressEntry& progress );
    double collection_state() const { return table.collection_state(); }
};

template <class Metric, class Lambda>
Fitter<Metric,Lambda>::Fitter( const Config& config, const input::Traits< engine::ImageStack >& traits, int images )
: window_cutter(config.fit_window_config, traits, std::set<int>(), 0),
  traits(traits), table( config, traits.fluorophores.size(), traits.plane_count() > 1, images * traits.plane_count() )
{
    DEBUG("Creating form fitter");
}

template <class Metric, class Lambda>
bool Fitter<Metric,Lambda>::
add_image( const engine::ImageStack& image, const Localization& position, int fluorophore ) 
{
    guf::Spot spot_position;
    spot_position.x() = position.position_x() / (1E-6 * si::meter);
    spot_position.y() = position.position_y() / (1E-6 * si::meter);
    fit_window::PlaneStack stack = window_cutter.cut_region_of_interest(image, spot_position);
    for (int i = 0; i < image.plane_count(); ++i) {
        DEBUG("Adding layer " << i << " of " << image.plane_count() << " to model with " << evaluators.size()
                << " evaluators");
        if ( ! table.needs_more_planes() ) return true;

        std::auto_ptr<PlaneFunction> new_evaluator( new PlaneFunction(stack[i]) );

        LocalizationValueFinder iv(fluorophore, traits.optics(i), position, i);
        iv.find_values( new_evaluator->get_gaussian() );
        iv.find_values( new_evaluator->get_background() );
        new_evaluator->get_gaussian().allow_leaving_ROI( true );

        /* After adding the evaluator to the table and the combiner, it is only
         * kept for later reference with the result() function. */
        table.add_plane( i, fluorophore );
        evaluators.push_back( new_evaluator );
    }
    return ! table.needs_more_planes();
}

template <class Metric, typename Lambda>
struct Fitter<Metric,Lambda>::less_amplitude
: public std::binary_function<bool,const PlaneFunction&,const PlaneFunction&>
{
    bool operator()( const PlaneFunction& a, const PlaneFunction& b ) {
        return gaussian_kernel( a )( gaussian_psf::Amplitude() ) < gaussian_kernel( b )( gaussian_psf::Amplitude() );
    }
};

template <>
void Fitter<plane::negative_poisson_likelihood,gaussian_psf::No3D>::apply_z_calibration() {}
template <>
void Fitter<plane::squared_deviations,gaussian_psf::No3D>::apply_z_calibration() {}
template <>
void Fitter<plane::negative_poisson_likelihood,gaussian_psf::DepthInfo3D>::apply_z_calibration() {}
template <>
void Fitter<plane::squared_deviations,gaussian_psf::DepthInfo3D>::apply_z_calibration() {}
template <class Metric, class Lambda>
void Fitter<Metric,Lambda>::apply_z_calibration()
{
    typename Evaluators::iterator highest_amp = std::max_element( 
        evaluators.begin(), evaluators.end(), less_amplitude() );
    double focus_z = gaussian_kernel( *highest_amp )( gaussian_psf::MeanZ() );
    BOOST_FOREACH( PlaneFunction& f, evaluators ) {
        /* Deviate minimally from the ideal Z position to avoid Z equalities */
        if ( table.is_layer_independent( gaussian_psf::ZPosition<0>() ) )
            gaussian_kernel( f )( gaussian_psf::ZPosition<0>() ) = focus_z - 1E-4;
        if ( table.is_layer_independent( gaussian_psf::ZPosition<1>() ) )
            gaussian_kernel( f )( gaussian_psf::ZPosition<1>() ) = focus_z + 1E-4;
    }
}

template <class Metric, class Lambda>
void Fitter<Metric,Lambda>::fit( input::Traits< engine::ImageStack >& new_traits, simparm::ProgressEntry& progress ) 
{
    apply_z_calibration();
    progress.indeterminate = true;
    progress.setValue( 0.5 );

    CombinedFunction combiner( table.get_reduction_matrix() );
    combiner.set_fitters( evaluators.begin(), evaluators.end() );

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
    if ( config.mle() )
        return std::auto_ptr<FittingVariant>( new Fitter< plane::negative_poisson_likelihood, Lambda > ( config, traits, images ) );
    else
        return std::auto_ptr<FittingVariant>( new Fitter< plane::squared_deviations, Lambda > ( config, traits, images ) );
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
