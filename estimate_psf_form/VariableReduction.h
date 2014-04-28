#ifndef DSTORM_ESTIMATE_PSF_FORM_VARIABLE_REDUCTION_H
#define DSTORM_ESTIMATE_PSF_FORM_VARIABLE_REDUCTION_H

#include <vector>

#include "calibrate_3d/constant_parameter.hpp"
#include "constant_background/model.hpp"
#include "estimate_psf_form/Config.h"
#include "gaussian_psf/fixed_form.h"
#include "gaussian_psf/parameters.h"
#include "gaussian_psf/is_plane_dependent.h"
#include "nonlinfit/make_bitset.h"
#include "nonlinfit/sum/VariableMap.hpp"

#include "debug.h"

namespace dStorm {
namespace estimate_psf_form {

class vanishes_when_circular
{
    const Config& config;
public:
    vanishes_when_circular( const Config& config ) : config(config) {}
    typedef bool result_type;

    bool operator()( gaussian_psf::ZPosition<1> ) { return ! config.astigmatism(); }
    bool operator()( gaussian_psf::BestSigma<1> ) { return config.symmetric(); }
    template <int Term>
    bool operator()( gaussian_psf::DeltaSigma<1,Term> ) { return config.symmetric(); }

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

    nonlinfit::sum::VariableMap result;
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
    VariableReduction( const Config& config, int fluorophore_count, bool multiplane, int nop );
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
    const nonlinfit::sum::VariableMap& get_reduction_matrix() const
        { return result; }
    template <typename Parameter>
    bool is_layer_independent( Parameter );

    double collection_state() const { return double(plane_count) / max_plane_count; }
};

template <typename Variables>
template <typename Parameter>
bool VariableReduction<Variables>::is_layer_independent( Parameter p ) {
    return gaussian_psf::is_plane_independent(config.laempi_fit(),config.disjoint_amplitudes(), config.universal_best_sigma(), config.universal_3d())(p);
}

struct is_positional {
    typedef bool result_type;
    bool operator()( constant_background::Amount ) { return true; }
    template <typename Parameter>
    bool operator()( Parameter ) {
        return gaussian_psf::FixedForm::apply< Parameter >::type::value;
    }
};

template <typename Variables>
VariableReduction<Variables>::VariableReduction( const Config& config, int fluorophore_count, bool multiplane, int nop )
: config(config), 
  first_fluorophore_occurence( fluorophore_count, -1 ),
  plane_count(0), max_plane_count(nop), result(VariableCount)
{
    positional = make_bitset( Variables(), is_positional() );
    layer_dependent = make_bitset( Variables(), 
        gaussian_psf::is_plane_independent(config.laempi_fit(),config.disjoint_amplitudes(), config.universal_best_sigma()) );
    layer_dependent.flip();
    fluorophore_dependent = make_bitset( Variables(), gaussian_psf::is_fluorophore_dependent() );
    merged = make_bitset( Variables(), vanishes_when_circular(config) );
    constant = make_bitset( Variables(), calibrate_3d::constant_parameter( multiplane, config, true ) );
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

}
}

#endif
