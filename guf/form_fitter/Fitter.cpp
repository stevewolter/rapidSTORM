#include "decl.h"
#include <Eigen/StdVector>
#include <boost/foreach.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "Fitter.h"
#include "Config.h"
#include <dStorm/image/slice.h>
#include <dStorm/Localization.h>
#include <dStorm/engine/JobInfo.h>
#include "guf/guf/TransformedImage.hpp"
#include "guf/guf/Spot.h"
#include <nonlinfit/plane/Distance.hpp>
#include <nonlinfit/Bind.h>
#include <nonlinfit/sum/AbstractFunction.hpp>
#include <nonlinfit/sum/AbstractMap.hpp>
#include <nonlinfit/sum/Evaluator.h>
#include <nonlinfit/VectorPosition.hpp>
#include <nonlinfit/make_bitset.h>
#include <nonlinfit/make_functor.hpp>
#include "guf/psf/is_plane_dependent.h"
#include "guf/psf/Zhuang.h"
#include "guf/psf/No3D.h"
#include "guf/psf/fixed_form.h"
#include "guf/psf/StandardFunction.h"
#include "guf/psf/JointEvaluator.h"
#include <nonlinfit/levmar/Fitter.hpp>
#include "guf/guf/Statistics.h"
#define BOOST_DETAIL_CONTAINER_FWD_HPP
#include <boost/lambda/lambda.hpp>
#include <boost/variant/get.hpp>
#include <nonlinfit/BoundFunction.hpp>
#include "guf/guf/mle_converter.h"

#include <nonlinfit/terminators/RelativeChange.h>
#include <nonlinfit/terminators/StepLimit.h>
#include <nonlinfit/terminators/All.h>

#include "LocalizationValueFinder.h"

namespace dStorm {
namespace form_fitter {

namespace PSF = dStorm::guf::PSF;

using namespace nonlinfit;

struct OnePlaneAssignment
{
    template <typename Type> struct apply { typedef boost::mpl::true_ type; };
};
template <> struct OnePlaneAssignment::apply< guf::PSF::Prefactor > { typedef boost::mpl::false_ type; };
template <> struct OnePlaneAssignment::apply< guf::PSF::Wavelength > { typedef boost::mpl::false_ type; };
template <> struct OnePlaneAssignment::apply< guf::PSF::ZPosition > { typedef boost::mpl::false_ type; };
template <int Dim> struct OnePlaneAssignment::apply< guf::PSF::ZOffset<Dim> > { typedef boost::mpl::false_ type; };
template <int Dim> struct OnePlaneAssignment::apply< nonlinfit::Xs<Dim,PSF::LengthUnit> > { typedef boost::mpl::false_ type; };

struct MultiPlaneAssignment
{
    template <typename Type> struct apply
        { typedef typename OnePlaneAssignment::apply<Type>::type type; };
};
template <> struct MultiPlaneAssignment::apply< guf::PSF::Prefactor > { typedef boost::mpl::true_ type; };

struct vanishes_when_circular
{
    typedef bool result_type;

    bool operator()( PSF::BestSigma<1> ) { return true; }
    bool operator()( PSF::DeltaSigma<1> ) { return true; }

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

    std::bitset< VariableCount > 
        positional, /**< Indicates per-fluorophore parameters like amplitude,
                         position or shift. Indexed by variable number. */
        layer_dependent, /**< E.g. shift or z offset. */
        fluorophore_dependent, /**< Variables depending on fluorophore type,
                                    e.g. transmission coefficients. */
        vanishes /**< Parameters that are redundant for circular PSFs */;
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
    VariableReduction( const Config& config, const input::Traits< engine::Image >& traits, int nop );
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
};

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
VariableReduction<Lambda>::VariableReduction( const Config& config, const input::Traits< engine::Image >& traits, int nop )
: first_fluorophore_occurence( traits.fluorophores.size(), -1 ),
  plane_count(0), max_plane_count(nop)
{
    positional = make_bitset( Variables(), is_positional() );
    assert( positional.any() );
    layer_dependent = make_bitset( Variables(), 
        guf::PSF::is_plane_independent(config.laempi_fit(),config.disjoint_amplitudes()) );
    layer_dependent.flip();
    fluorophore_dependent = make_bitset( Variables(), guf::PSF::is_fluorophore_dependent() );
    vanishes = make_bitset( Variables(), vanishes_when_circular() );
    if ( ! config.circular_psf() ) vanishes.reset();
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
VariableReduction<Lambda>::reducer::operator()( const int function, const int j ) const
{
    int base_row = function,
        base_col = j;
    int my_layer = layer;
    if ( ! r.layer_dependent[j] ) {
        /* Plane-independent parameters can be reduced to the first plane. */
        base_row -= layer; 
        my_layer = 0;
    }
    if ( ! r.positional[j] && ! r.fluorophore_dependent[j] ) {
        /* The parameter is common to all fluorophores regardless of type.
            * Reduce to the matching plane of the first fluorophore. */
            base_row = my_layer;
    }
    if (  ! r.positional[j] && r.fluorophore_dependent[j] ) {
        /* The parameter is common to all fluorophores of this type. Locate
            * the first instance of the current fluorophore type and reduce to it. */
        base_row = r.first_fluorophore_occurence[ fluorophore_type ] + my_layer;
    }
    if ( r.vanishes[j] )
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
    typedef typename PSF::StandardFunction< Lambda, 1 >::type TheoreticalFunction;
    typedef BoundFunction< 
        nonlinfit::plane::Distance<
            TheoreticalFunction,
            plane::xs_joint<double,PSF::LengthUnit,2>::type,
            Metric > > 
        PlaneFunction;
    typedef nonlinfit::VectorPosition< Lambda > VectorPosition;
    typedef sum::AbstractFunction< PlaneFunction, PlaneFunction, Eigen::Dynamic > CombinedFunction;
    typedef guf::TransformedImage< PSF::LengthUnit > Transformation;

    /** Transformations indexed by input layer. */
    boost::ptr_vector<Transformation> transformations;
    /** Transformations indexed by input plane. */
    boost::ptr_vector< PlaneFunction > evaluators;
    const dStorm::engine::InputTraits& traits;
    VariableReduction<TheoreticalFunction> table;

    /** Get one of the model instances matching the given fluorophore type and layer. */
    const Lambda& result( int fluorophore = -1, int layer = 0 ) {
        const int i = ( fluorophore == -1 ) ? layer : table.find_plane(layer, fluorophore);
        assert( i >= 0 && i < int(evaluators.size()) );
        return evaluators[i].get_expression().get_part( boost::mpl::int_<0>() );
    }

    dStorm::traits::Optics<3>::DepthInfo get_3d( const PSF::Zhuang& m ) {
        traits::Zhuang3D three_d;
        for (int i = 0; i < 2; ++i)
            three_d.widening[i] = quantity<traits::Zhuang3D::Unit>( 
                m.get< PSF::DeltaSigma >(i) * m( PSF::Wavelength() ) );
        return three_d;
    }

    dStorm::traits::Optics<3>::DepthInfo get_3d( const PSF::No3D& ) {
        return traits::No3D();
    }


  public:
    /** \see FittingVariant::create(). */
    Fitter( const Config& config, const input::Traits< engine::Image >& traits, int images );
    /** \see FittingVariant::add_image(). */
    bool add_image( const engine::Image& image, const Localization& position, int fluorophore );
    /** \see FittingVariant::fit(). */
    void fit( input::Traits< engine::Image >& new_traits );
};

template <class Metric, class Lambda>
Fitter<Metric,Lambda>::Fitter( const Config& config, const input::Traits< engine::Image >& traits, int images )
: traits(traits), table( config, traits, images * traits.plane_count() )
{
    DEBUG("Creating form fitter");
    guf::Spot max_distance;
    for (int i = 0; i < max_distance.rows(); ++i)
        max_distance[i] = (*traits.psf_size())[i] * 5.0f;
    for ( int i = 0; i < traits.plane_count(); ++i ) {
        transformations.push_back( new Transformation(max_distance, traits.plane(i)) );
    }

}

template <class Metric, class Lambda>
bool Fitter<Metric,Lambda>::
add_image( const engine::Image& image, const Localization& position, int fluorophore ) 
{
    for (int i = 0; i < image.depth_in_pixels(); ++i) {
        DEBUG("Adding layer " << i << " of " << image.depth_in_pixels() << " to model with " << combiner.evaluator_count()
                << " evaluators");
        if ( ! table.needs_more_planes() ) return true;

        std::auto_ptr<PlaneFunction> new_evaluator( new PlaneFunction() );
        /* The data are persisted in the evaluator's data store. */
        guf::Statistics<2> stats = 
            transformations[ i ].set_data( 
                new_evaluator->get_data(),
                image.slice(2, i * camera::pixel),
                guf::Spot( position.position().template head<2>() ),
                guf::mle_converter(traits.plane(i))  );
        /* Check the number pixels used in PSF estimation for this spot & plane. */
        assert ( stats.pixel_count > 0 );

        dStorm::engine::JobInfo info( 0, 0 * camera::ad_count, traits, fluorophore );
        LocalizationValueFinder iv(info, traits.plane(i), position, i);
        iv.find_values( new_evaluator->get_expression().get_part( boost::mpl::int_<0>() ) );
        iv.find_values( new_evaluator->get_expression().get_part( boost::mpl::int_<1>() ) );

        /* After adding the evaluator to the table and the combiner, it is only
         * kept for later reference with the result() function. */
        table.add_plane( i, fluorophore );
        evaluators.push_back( new_evaluator );
    }
    return ! table.needs_more_planes();
}
    
template <class Metric, class Lambda>
void Fitter<Metric,Lambda>::fit( input::Traits< engine::Image >& new_traits ) 
{
    CombinedFunction combiner( table.get_reduction_matrix() );
    combiner.set_fitters( evaluators.begin(), evaluators.end() );

    nonlinfit::levmar::Fitter fitter = nonlinfit::levmar::Config();
    nonlinfit::terminators::StepLimit terminator(300);
    fitter.fit( combiner, combiner, terminator );

    for (size_t i = 0; i < traits.fluorophores.size(); ++i) {
        if ( ! table.has_fluorophore( i ) ) {
            std::cerr << "Have seen no examples of fluorophore " << i << " and left its parameters unchanged." << std::endl;
            continue;
        }
        new_traits.fluorophores.at(i).wavelength = quantity<si::length>( result(i)( PSF::Wavelength() ) );
        float target_transmission = 0, total_transmission = 0; 
        for (int j = 0; j < traits.plane_count(); ++j) {
            total_transmission += result(i,j)( PSF::Prefactor() );
            target_transmission += traits.plane(j).transmission_coefficient(i);
        }
        for (int j = 0; j < traits.plane_count(); ++j) {
            new_traits.plane(j).set_fluorophore_transmission_coefficient(i, 
                result(i,j)( PSF::Prefactor() )
                    * target_transmission / total_transmission );
        }
    }
    for (int i = 0; i < 2; ++i)
        (*new_traits.psf_size())[i] = quantity<si::length>(
            result().template get< PSF::BestSigma >(i) * new_traits.fluorophores.at(0).wavelength * 1.075);
    new_traits.depth_info = get_3d( result() );
}

/** Helper for FittingVariant::create() that instantiates a Fitter class. */
template <class Lambda>
std::auto_ptr<FittingVariant>
create2( const Config& config, const input::Traits< engine::Image >& traits, int images )
{
    if ( config.mle() )
        return std::auto_ptr<FittingVariant>( new Fitter< plane::negative_poisson_likelihood, Lambda > ( config, traits, images ) );
    else
        return std::auto_ptr<FittingVariant>( new Fitter< plane::squared_deviations, Lambda > ( config, traits, images ) );
}

/** Helper for FittingVariant::create() that selects an assignment based on 
 *  the number of layers. */
template <typename Expression>
std::auto_ptr<FittingVariant>
create1( const Config& config, const input::Traits< engine::Image >& traits, int images )
{
    if ( traits.plane_count() > 1 )
        return create2< nonlinfit::Bind<Expression, MultiPlaneAssignment> >( config, traits, images );
    else
        return create2< nonlinfit::Bind<Expression, OnePlaneAssignment> >( config, traits, images );
}

std::auto_ptr<FittingVariant>
FittingVariant::create( const Config& config, const input::Traits< engine::Image >& traits, int images )
{
    std::auto_ptr<FittingVariant> rv;
    bool has_3d = boost::get< traits::Zhuang3D >( traits.depth_info.get_ptr() ) != NULL;
    if ( has_3d )
        return create1< PSF::Zhuang >( config, traits, images );
    else
        return create1< PSF::No3D >( config, traits, images );
}

}
}
