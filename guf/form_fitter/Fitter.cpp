#include "decl.h"
#include <Eigen/StdVector>
#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/variant/apply_visitor.hpp>
#include "Fitter.h"
#include "Config.h"
#include <dStorm/image/slice.h>
#include <dStorm/Localization.h>
#include <dStorm/engine/JobInfo.h>
#include "guf/guf/Spot.h"
#include <nonlinfit/plane/Distance.hpp>
#include <nonlinfit/plane/JointData.hpp>
#include <nonlinfit/Bind.h>
#include <nonlinfit/sum/AbstractFunction.hpp>
#include <nonlinfit/sum/AbstractMap.hpp>
#include <nonlinfit/sum/Evaluator.h>
#include <nonlinfit/VectorPosition.hpp>
#include <nonlinfit/make_bitset.h>
#include <nonlinfit/make_functor.hpp>
#include "guf/psf/is_plane_dependent.h"
#include "guf/psf/Polynomial3D.h"
#include "guf/psf/Spline3D.h"
#include "guf/psf/No3D.h"
#include "guf/psf/fixed_form.h"
#include "guf/psf/StandardFunction.h"
#include "guf/psf/JointEvaluator.h"
#include <nonlinfit/levmar/Fitter.hpp>
#define BOOST_DETAIL_CONTAINER_FWD_HPP
#include <boost/lambda/lambda.hpp>
#include <boost/variant/get.hpp>
#include <nonlinfit/BoundFunction.hpp>
#include "guf/guf/mle_converter.h"
#include <dStorm/engine/InputTraits.h>
#include <fstream>
#include "guf/select_3d_lambda.hpp"

#include "guf/guf/Optics.h"
#include "guf/guf/DataPlaneImpl.hpp"

#include <nonlinfit/terminators/RelativeChange.h>
#include <nonlinfit/terminators/StepLimit.h>
#include <nonlinfit/terminators/All.h>
#include <dStorm/threed_info/Polynomial3D.h>
#include <dStorm/threed_info/No3D.h>
#include <dStorm/threed_info/Spline3D.h>

#include "LocalizationValueFinder.h"
#include "calibrate_3d/constant_parameter.hpp"

namespace dStorm {
namespace form_fitter {

namespace PSF = dStorm::guf::PSF;

using namespace nonlinfit;

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

    template <class SubFunction, typename Base>
    bool operator()( nonlinfit::TermParameter<SubFunction,Base> ) { return operator()(Base()); }

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
 *  \tparam _Lambda The nonlinfit lambda that is used as the MultiPlaneEvaluator's input 
 **/
template <typename _Lambda>
class VariableReduction 
{
    typedef _Lambda Lambda;
    typedef typename Lambda::Variables Variables;
    static const int VariableCount = boost::mpl::size< Variables >::value;

    const Config config;
    std::bitset< VariableCount > 
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

    sum::AbstractMap< VariableCount > result;
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
    const sum::AbstractMap< VariableCount >& get_reduction_matrix() const
        { return result; }
    template <typename Parameter>
    bool is_layer_independent( Parameter );

    double collection_state() const { return double(plane_count) / max_plane_count; }
};

template <typename Lambda>
template <typename Parameter>
bool VariableReduction<Lambda>::is_layer_independent( Parameter p ) {
    return guf::PSF::is_plane_independent(config.laempi_fit(),config.disjoint_amplitudes(), config.universal_best_sigma(), config.universal_3d())(p);
}

struct is_positional {
    typedef bool result_type;
    template <typename Func, typename Base>
    bool operator()( TermParameter< Func, Base > ) { return (*this)( Base() ); }
    bool operator()( constant_background::Amount ) { return true; }
    template <typename Parameter>
    bool operator()( Parameter ) {
        return guf::PSF::FixedForm::apply< Parameter >::type::value;
    }
};

template <typename Lambda>
VariableReduction<Lambda>::VariableReduction( const Config& config, const input::Traits< engine::ImageStack >& traits, int nop )
: config(config), 
  first_fluorophore_occurence( traits.fluorophores.size(), -1 ),
  plane_count(0), max_plane_count(nop)
{
    positional = make_bitset( Variables(), is_positional() );
    assert( positional.any() );
    layer_dependent = make_bitset( Variables(), 
        guf::PSF::is_plane_independent(config.laempi_fit(),config.disjoint_amplitudes(), config.universal_best_sigma()) );
    layer_dependent.flip();
    fluorophore_dependent = make_bitset( Variables(), guf::PSF::is_fluorophore_dependent() );
    merged = make_bitset( Variables(), vanishes_when_circular(config) );
    constant = make_bitset( Variables(), calibrate_3d::constant_parameter( traits.plane_count() > 1, config ) );
}

template <typename Lambda>
void VariableReduction<Lambda>::add_plane( const int layer, const int fluorophore_type )
{
    const int i = plane_count++;
    assert( plane_count <= max_plane_count );

    if ( first_fluorophore_occurence[ fluorophore_type ] == -1 )
        first_fluorophore_occurence[ fluorophore_type ] = i;

    result.add_function( reducer(*this, fluorophore_type, layer) );
}

template <typename Lambda>
std::pair<int,int>
VariableReduction<Lambda>::reducer::operator()( const int function, const int parameter ) const
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

template <typename Lambda>
int VariableReduction<Lambda>::find_plane( const int layer, const int fluorophore )
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

    typedef typename PSF::StandardFunction< Lambda, 1 >::type TheoreticalFunction;
    typedef plane::xs_joint<double,PSF::LengthUnit,2>::type DataTag;
    typedef BoundFunction< 
        nonlinfit::plane::Distance< TheoreticalFunction, DataTag, Metric > > 
        PlaneFunction;
    typedef nonlinfit::VectorPosition< Lambda > VectorPosition;
    typedef sum::AbstractFunction< PlaneFunction, PlaneFunction, nonlinfit::sum::VariableDropPolicy > CombinedFunction;

    /** Optics indexed by input layer. */
    boost::ptr_vector<guf::Optics> optics;
    typedef boost::ptr_vector< PlaneFunction > Evaluators;
    Evaluators evaluators;
    const dStorm::engine::InputTraits& traits;
    VariableReduction<TheoreticalFunction> table;
    const double width_correction;

    /** Get one of the model instances matching the given fluorophore type and layer. */
    const Lambda& result( int fluorophore = -1, int layer = 0 ) {
        const int i = ( fluorophore == -1 ) ? layer : table.find_plane(layer, fluorophore);
        assert( i >= 0 && i < int(evaluators.size()) );
        return evaluators[i].get_expression().get_part( boost::mpl::int_<0>() );
    }

    boost::shared_ptr<const threed_info::DepthInfo> get_3d( const PSF::Polynomial3D& m, int plane, Direction dir ) {
        boost::shared_ptr<threed_info::Polynomial3D> three_d( 
            new threed_info::Polynomial3D(dynamic_cast<const threed_info::Polynomial3D&>(*traits.optics(plane).depth_info(dir)) ) );
        three_d->set_focal_plane( threed_info::ZPosition( m.get< PSF::ZPosition >(dir) ) );
        three_d->set_base_width( threed_info::Sigma( m.get< PSF::BestSigma >(dir) * width_correction ) );
        for (int term = threed_info::Polynomial3D::MinTerm; term <= threed_info::Polynomial3D::Order; ++term) {
            three_d->set_slope( term, threed_info::Polynomial3D::WidthSlope( m.get_delta_sigma(dir,term) ) );
        }
        return three_d;
    }

    boost::shared_ptr<const threed_info::DepthInfo> get_3d( const PSF::Spline3D& s, int plane, Direction dir ) {
        return traits.optics(plane).depth_info(dir);
    }

    boost::shared_ptr<const threed_info::DepthInfo> get_3d( const PSF::No3D& m, int plane, Direction dir ) {
        boost::shared_ptr<threed_info::No3D> rv( new threed_info::No3D() );
        rv->sigma = threed_info::Sigma( m.get< PSF::BestSigma >(dir) * width_correction );
        return rv;
    }

    void apply_z_calibration();
    static Lambda& gaussian_kernel( PlaneFunction& e ) 
        { return e.get_expression().get_part( boost::mpl::int_<0>() ); }
    static const Lambda& gaussian_kernel( const PlaneFunction& e ) 
        { return e.get_expression().get_part( boost::mpl::int_<0>() ); }

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
: traits(traits), table( config, traits, images * traits.plane_count() ), width_correction( config.width_correction() )
{
    DEBUG("Creating form fitter");
    for ( int i = 0; i < traits.plane_count(); ++i ) {
        optics.push_back( new guf::Optics(config.fit_window_width().template cast< guf::Spot::Scalar >(), traits.plane(i)) );
    }
}

template <class Metric, class Lambda>
bool Fitter<Metric,Lambda>::
add_image( const engine::ImageStack& image, const Localization& position, int fluorophore ) 
{
    for (int i = 0; i < image.plane_count(); ++i) {
        DEBUG("Adding layer " << i << " of " << image.plane_count() << " to model with " << evaluators.size()
                << " evaluators");
        if ( ! table.needs_more_planes() ) return true;

        guf::DataPlaneImpl<DataTag>
            data_creator( optics[i], image.plane(i), 
               guf::Spot( position.position().template head<2>() ) );
        std::auto_ptr<PlaneFunction> new_evaluator( new PlaneFunction() );
        new_evaluator->get_data() = data_creator.get_the_data();

        LocalizationValueFinder iv(fluorophore, traits.optics(i), position, i);
        iv.find_values( new_evaluator->get_expression().get_part( boost::mpl::int_<0>() ) );
        iv.find_values( new_evaluator->get_expression().get_part( boost::mpl::int_<1>() ) );
        gaussian_kernel( *new_evaluator ).allow_leaving_ROI( true );

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
void Fitter<plane::negative_poisson_likelihood,PSF::Spline3D>::apply_z_calibration() {}
template <>
void Fitter<plane::squared_deviations,PSF::Spline3D>::apply_z_calibration() {}
template <class Metric, class Lambda>
void Fitter<Metric,Lambda>::apply_z_calibration()
{
    typename Evaluators::iterator highest_amp = std::max_element( 
        evaluators.begin(), evaluators.end(), less_amplitude() );
    quantity<PSF::MeanZ::Unit> focus_z = gaussian_kernel( *highest_amp )( PSF::MeanZ() );
    BOOST_FOREACH( PlaneFunction& f, evaluators ) {
        /* Deviate minimally from the ideal Z position to avoid Z equalities */
        if ( table.is_layer_independent( PSF::ZPosition<0>() ) )
            gaussian_kernel( f )( PSF::ZPosition<0>() ) = focus_z - quantity<PSF::MeanZ::Unit>(1E-10 * si::meters);
        if ( table.is_layer_independent( PSF::ZPosition<1>() ) )
            gaussian_kernel( f )( PSF::ZPosition<1>() ) = focus_z + quantity<PSF::MeanZ::Unit>(1E-10 * si::meters);
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
    nonlinfit::terminators::StepLimit terminator(300);
    fitter.fit( combiner, combiner, 
        all( nonlinfit::terminators::StepLimit(300), 
#ifdef VERBOSE_STATE
        all( print_state(), nonlinfit::terminators::RelativeChange(1E-4) )
#else
        nonlinfit::terminators::RelativeChange(1E-4)
#endif
        ) );

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
    if ( dynamic_cast< const threed_info::Polynomial3D* >(d) )
        return create2<PSF::Polynomial3D,threed_info::Polynomial3D>( config, traits, images );
    else if ( dynamic_cast< const threed_info::No3D* >(d) )
        return create2<PSF::No3D,threed_info::No3D>( config, traits, images );
    else if ( dynamic_cast< const threed_info::Spline3D* >(d) )
        return create2<PSF::Spline3D,threed_info::Spline3D>( config, traits, images );
    else
        throw std::logic_error("Missing 3D model in form fitter");
}

}
}
