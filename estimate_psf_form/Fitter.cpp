#include <deque>
#include <fstream>

#include "decl.h"
#include "estimate_psf_form/decl.h"
#include <Eigen/StdVector>
#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/variant/apply_visitor.hpp>
#include "estimate_psf_form/Fitter.h"
#include "estimate_psf_form/Config.h"
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
#include "nonlinfit/sum/VariableMap.hpp"
#include "nonlinfit/make_bitset.h"
#include "nonlinfit/make_functor.hpp"
#include "gaussian_psf/parameters.h"
#include "gaussian_psf/is_plane_dependent.h"
#include "gaussian_psf/DepthInfo3D.h"
#include "gaussian_psf/No3D.h"
#include "gaussian_psf/fixed_form.h"
#include "gaussian_psf/JointEvaluator.h"
#include "nonlinfit/levmar/Fitter.h"
#define BOOST_DETAIL_CONTAINER_FWD_HPP
#include <boost/lambda/lambda.hpp>
#include <boost/variant/get.hpp>
#include "engine/InputTraits.h"

#include "fit_window/chunkify.h"
#include "fit_window/Optics.h"
#include "fit_window/FitWindowCutter.h"

#include <nonlinfit/terminators/StepLimit.h>
#include "threed_info/No3D.h"
#include "threed_info/Spline3D.h"

#include "estimate_psf_form/LocalizationValueFinder.h"
#include "calibrate_3d/constant_parameter.hpp"
#include "constant_background/model.hpp"

#include "debug.h"

namespace dStorm {
namespace estimate_psf_form {

namespace PSF = dStorm::gaussian_psf;

using namespace nonlinfit;

struct NonDataParameters
{
    template <typename Type> struct apply { typedef boost::mpl::true_ type; };
};
template <> struct NonDataParameters::apply< gaussian_psf::XPosition > {typedef boost::mpl::false_ type; };
template <> struct NonDataParameters::apply< gaussian_psf::YPosition > {typedef boost::mpl::false_ type; };

struct print_state {
	void matrix_is_unsolvable(){}
        template <typename Position>
        void improved( const Position& current, const Position& shift )
            { std::cerr << current.template head<15>().transpose() << std::endl; }
        void failed_to_improve( bool ) {}
        bool should_continue_fitting() const { return true; }
};

class vanishes_when_circular
{
    const Config& config;
public:
    vanishes_when_circular( const Config& config ) : config(config) {}
    typedef bool result_type;

    bool operator()( PSF::ZPosition<1> ) { return ! config.astigmatism(); }
    bool operator()( PSF::BestSigma<1> ) { return config.symmetric(); }
    template <int Term>
    bool operator()( PSF::DeltaSigma<1,Term> ) { return config.symmetric(); }

    template <typename Parameter> 
    bool operator()( Parameter ) { return false; }
};

/** \brief Creates a reduction matrix for multi-plane, multi-fluorophore datasets
 *  
 *  This class is used to create a reduction matrix for use with nonlinfit::plane::MultiPlaneEvaluator
 *  for the Fitter class. The matrix is produced for n planes, with n given in the constructor, from
 *  the n calls to add_plane(). After the construction phase, the matrix can be retrieved with
 *  get_reduction_matrix() and the variable count with get_variable_count().
 *
 *  In the context of this class, a fluorophore is one fluorescent entity. It can be active
 *  in multiple layers (e.g. 2 for dual-color), and the layers of all fluorophores form the
 *  planes. For example, given 50 fluorophores on 2 layers, we have 100 planes.
 *
 *  \tparam Variables_ The variable tag vector for the MultiPlaneEvaluator's input 
 **/
template <typename Variables>
class VariableReduction 
{
    static const int VariableCount = boost::mpl::size< Variables >::value;

    const Config config;
    std::vector<bool> 
        positional, /**< Indicates per-fluorophore parameters like amplitude,
                         position or shift. Indexed by variable number. */
        layer_dependent, /**< E.g. shift or z offset. */
        fluorophore_dependent, /**< Variables depending on fluorophore type,
                                    e.g. transmission coefficients. */
        merged       /**< Parameters that are redundant for circular PSFs */,
        constant     /**< Parameter is runtime-determined constant. */;
    /** The plane numbers for each fluorophore type's first occurence. */
    std::vector<int> first_fluorophore_occurence;
    int plane_count;
    const int max_plane_count;

    sum::VariableMap result;
    struct reducer {
        const VariableReduction& r;
        int fluorophore_type, layer;
        reducer( const VariableReduction& r, int fluorophore, int layer )
            : r(r), fluorophore_type(fluorophore), layer(layer) {}
        typedef std::pair<int,int> result_type;
        std::pair<int,int> operator()( int plane, int fluorophore ) const;
    };

  public:
    /** Constructor. Object calls aside from add_plane will not be valid until
     *  add_plane() has been called for nop times.
     *
     *  @param nop Number of planes to generate a matrix for. */
    VariableReduction( const Config& config, const input::Traits< engine::ImageStack >& traits, int nop );
    void add_plane( const int layer, const int fluorophore );
    /** Find the first plane that has been adding with matching parameters. */
    inline int find_plane( const int layer, const int fluorophore );
    /** Tests whether any plane with the given fluorophore type has
     *  been added. */
    bool has_fluorophore( const int fluorophore ) 
        { return first_fluorophore_occurence[fluorophore] != -1; }

    bool needs_more_planes() const { return plane_count < max_plane_count; }
    /** Get the result matrix, which is a valid input matrix for 
     *  nonlinfit::plane::MultiPlaneEvaluator if add_plane() has been called
     *  sufficiently often. */
    const sum::VariableMap& get_reduction_matrix() const
        { return result; }
    template <typename Parameter>
    bool is_layer_independent( Parameter );

    double collection_state() const { return double(plane_count) / max_plane_count; }
};

template <typename Variables>
template <typename Parameter>
bool VariableReduction<Variables>::is_layer_independent( Parameter p ) {
    return PSF::is_plane_independent(config.laempi_fit(),config.disjoint_amplitudes(), config.universal_best_sigma(), config.universal_3d())(p);
}

struct is_positional {
    typedef bool result_type;
    template <typename Func, typename Base>
    bool operator()( constant_background::Amount ) { return true; }
    template <typename Parameter>
    bool operator()( Parameter ) {
        return PSF::FixedForm::apply< Parameter >::type::value;
    }
};

template <typename Variables>
VariableReduction<Variables>::VariableReduction( const Config& config, const input::Traits< engine::ImageStack >& traits, int nop )
: config(config), 
  first_fluorophore_occurence( traits.fluorophores.size(), -1 ),
  plane_count(0), max_plane_count(nop), result(VariableCount)
{
    positional = make_bitset( Variables(), is_positional() );
    layer_dependent = make_bitset( Variables(), 
        PSF::is_plane_independent(config.laempi_fit(),config.disjoint_amplitudes(), config.universal_best_sigma()) );
    layer_dependent.flip();
    fluorophore_dependent = make_bitset( Variables(), PSF::is_fluorophore_dependent() );
    merged = make_bitset( Variables(), vanishes_when_circular(config) );
    constant = make_bitset( Variables(), calibrate_3d::constant_parameter( traits.plane_count() > 1, config, true ) );
}

template <typename Variables>
void VariableReduction<Variables>::add_plane( const int layer, const int fluorophore_type )
{
    const int i = plane_count++;
    assert( plane_count <= max_plane_count );

    if ( first_fluorophore_occurence[ fluorophore_type ] == -1 )
        first_fluorophore_occurence[ fluorophore_type ] = i;

    result.add_function( reducer(*this, fluorophore_type, layer) );
}

template <typename Variables>
std::pair<int,int>
VariableReduction<Variables>::reducer::operator()( const int function, const int parameter ) const
{
    int base_row = function,
        base_col = parameter;
    int my_layer = layer;
    if ( r.constant[parameter] ) 
        return std::make_pair(-1,-1);
    if ( ! r.layer_dependent[parameter] ) {
        /* Plane-independent parameters can be reduced to the first plane. */
        base_row -= layer; 
        my_layer = 0;
    }
    if ( ! r.positional[parameter] && ! r.fluorophore_dependent[parameter] ) {
        /* The parameter is common to all fluorophores regardless of type.
            * Reduce to the matching plane of the first fluorophore. */
            base_row = my_layer;
    }
    if (  ! r.positional[parameter] && r.fluorophore_dependent[parameter] ) {
        /* The parameter is common to all fluorophores of this type. Locate
            * the first instance of the current fluorophore type and reduce to it. */
        base_row = r.first_fluorophore_occurence[ fluorophore_type ] + my_layer;
    }
    if ( r.merged[parameter] )
        --base_col;
    return std::make_pair( base_row, base_col );
}

template <typename Variables>
int VariableReduction<Variables>::find_plane( const int layer, const int fluorophore )
{ 
    assert( first_fluorophore_occurence[fluorophore] != -1 );
    return first_fluorophore_occurence[fluorophore] + layer; 
}

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

    boost::shared_ptr<const threed_info::DepthInfo> get_3d( const PSF::DepthInfo3D& s, int plane, Direction dir ) {
        return traits.optics(plane).depth_info(dir);
    }

    boost::shared_ptr<const threed_info::DepthInfo> get_3d( const PSF::No3D& m, int plane, Direction dir ) {
        boost::shared_ptr<threed_info::No3D> rv( new threed_info::No3D() );
        rv->sigma = threed_info::Sigma( m.get< PSF::BestSigma >(dir) );
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
  traits(traits), table( config, traits, images * traits.plane_count() )
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
        return gaussian_kernel( a )( PSF::Amplitude() ) < gaussian_kernel( b )( PSF::Amplitude() );
    }
};

template <>
void Fitter<plane::negative_poisson_likelihood,PSF::No3D>::apply_z_calibration() {}
template <>
void Fitter<plane::squared_deviations,PSF::No3D>::apply_z_calibration() {}
template <>
void Fitter<plane::negative_poisson_likelihood,PSF::DepthInfo3D>::apply_z_calibration() {}
template <>
void Fitter<plane::squared_deviations,PSF::DepthInfo3D>::apply_z_calibration() {}
template <class Metric, class Lambda>
void Fitter<Metric,Lambda>::apply_z_calibration()
{
    typename Evaluators::iterator highest_amp = std::max_element( 
        evaluators.begin(), evaluators.end(), less_amplitude() );
    double focus_z = gaussian_kernel( *highest_amp )( PSF::MeanZ() );
    BOOST_FOREACH( PlaneFunction& f, evaluators ) {
        /* Deviate minimally from the ideal Z position to avoid Z equalities */
        if ( table.is_layer_independent( PSF::ZPosition<0>() ) )
            gaussian_kernel( f )( PSF::ZPosition<0>() ) = focus_z - 1E-4;
        if ( table.is_layer_independent( PSF::ZPosition<1>() ) )
            gaussian_kernel( f )( PSF::ZPosition<1>() ) = focus_z + 1E-4;
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
            total_transmission += result(i,j)( PSF::Prefactor() );
            target_transmission += traits.optics(j).transmission_coefficient(i);
        }
        for (int j = 0; j < traits.plane_count(); ++j) {
            new_traits.optics(j).set_fluorophore_transmission_coefficient(i, 
                result(i,j)( PSF::Prefactor() )
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
        return create2<PSF::No3D,threed_info::No3D>( config, traits, images );
    else if ( dynamic_cast< const threed_info::Spline3D* >(d) )
        return create2<PSF::DepthInfo3D,threed_info::Spline3D>( config, traits, images );
    else
        throw std::logic_error("Missing 3D model in form fitter");
}

}
}
