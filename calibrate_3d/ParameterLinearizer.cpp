#include <Eigen/StdVector>
#include "ParameterLinearizer.h"
#include "guf/psf/Polynomial3D.h"
#include <nonlinfit/Bind.h>
#include <nonlinfit/VectorPosition.hpp>
#include <nonlinfit/sum/AbstractMap.hpp>
#include <nonlinfit/sum/AbstractFunction.hpp>
#include <nonlinfit/make_bitset.h>
#include "Config.h"
#include "guf/psf/is_plane_dependent.h"
#include "guf/guf/TraitValueFinder.h"
#include <dStorm/engine/JobInfo.h>
#include <dStorm/threed_info/DepthInfo.h>
#include <boost/variant/get.hpp>
#include "constant_parameter.hpp"

namespace dStorm {
namespace calibrate_3d {

struct calibrated_parameter {
    template <typename Type> struct apply { typedef boost::mpl::false_ type; };
};
template <int Dim> struct calibrated_parameter::apply< guf::PSF::BestSigma<Dim> > 
    { typedef boost::mpl::true_ type; };
template <int Dim> struct calibrated_parameter::apply< guf::PSF::ZPosition<Dim> > 
    { typedef boost::mpl::true_ type; };
template <int Dim, int Term> struct calibrated_parameter::apply< guf::PSF::DeltaSigma<Dim,Term> > 
    { typedef boost::mpl::true_ type; };

class reducible_to_preceding_parameter {
    const bool is_symmetric, may_be_astigmatic;
public:
    reducible_to_preceding_parameter( const Config_& config )
        : is_symmetric( config.symmetric() ),
          may_be_astigmatic( config.astigmatism() ) {}

    bool operator()( guf::PSF::BestSigma<1> ) { return is_symmetric; }
    bool operator()( guf::PSF::ZPosition<1> ) { return ! may_be_astigmatic; }
    template <int Term>
    bool operator()( guf::PSF::DeltaSigma<1,Term> ) { return is_symmetric; }
    template <typename Parameter>
    bool operator()( Parameter ) { return false; }
};

template <typename Lambda>
class BoundMoveable {
    Lambda expression;
    nonlinfit::VectorPosition<Lambda> mover;
public:
    typedef nonlinfit::Evaluation< double, boost::mpl::size<typename Lambda::Variables>::value > Derivatives;
    typedef typename nonlinfit::VectorPosition<Lambda>::Position Position;

    BoundMoveable() : mover(expression) {}
    BoundMoveable( const BoundMoveable<Lambda>& o )
        : expression(o.expression), mover(expression) {}
    BoundMoveable& operator=( const BoundMoveable<Lambda>& o )
        { expression = o.expression; return *this; }
    Lambda& get_expression() { return expression; }
    const Lambda& get_expression() const { return expression; }
    void get_position( Position& p ) const { mover.get_position(p); }
    void set_position( const Position& p ) { mover.set_position(p); }
};

struct ParameterLinearizer::Pimpl 
{
    typedef nonlinfit::Bind< guf::PSF::Polynomial3D, calibrated_parameter > PSF;
    typedef BoundMoveable<PSF> OnePlane;
    static const int VariableCount = boost::mpl::size< PSF::Variables >::value;

    std::pair<int,int> reduce( const int plane, const int parameter ) const;

    std::bitset< VariableCount > reducible, plane_independent, constant;
    mutable std::vector< OnePlane, Eigen::aligned_allocator<OnePlane> > planes;
    typedef nonlinfit::sum::AbstractFunction< OnePlane, OnePlane, nonlinfit::sum::VariableDropPolicy > MultiPlane;
    mutable boost::optional< MultiPlane > multiplane;

public:
    Pimpl( const Config_& c );
    void set_plane_count( int plane_count );
    int variable_count() const { return multiplane->variable_count(); }
    Eigen::VectorXd linearize( const engine::InputTraits& ) const;
    void delinearize( const Eigen::VectorXd&, engine::InputTraits& into );
};

ParameterLinearizer::Pimpl::Pimpl( const Config_& config )
{
    reducible = nonlinfit::make_bitset( PSF::Variables(), 
        reducible_to_preceding_parameter(config) );
    plane_independent = nonlinfit::make_bitset( PSF::Variables(),
        guf::PSF::is_plane_independent(false, false, config.universal_best_sigma(), config.universal_3d()) );
    constant = nonlinfit::make_bitset( PSF::Variables(),
        constant_parameter(false, config) );
}

void ParameterLinearizer::Pimpl::set_plane_count( int plane_count )
{
    planes.resize( plane_count );
    nonlinfit::sum::AbstractMap< VariableCount > map;
    for (int i = 0; i < plane_count; ++i) 
    {
        map.add_function( boost::bind( &Pimpl::reduce, boost::ref(*this), _1, _2 ) );
    }

    multiplane = boost::in_place( boost::cref(map) );
    multiplane->set_fitters( planes.begin(), planes.end() );
}

std::pair<int,int> 
ParameterLinearizer::Pimpl::reduce( const int plane, const int parameter ) const
{
    if ( constant[ parameter ] )
        return std::make_pair(-1,-1);
    else {
        std::pair<int,int> rv( plane, parameter );
        if ( reducible[ parameter ] )
            rv.second -= 1;
        if ( plane_independent[ parameter ] )
            rv.first = 0;
        return rv;
    }
}

ParameterLinearizer::ParameterLinearizer( const Config_& c ) 
: pimpl( new Pimpl(c) ) {}

ParameterLinearizer::~ParameterLinearizer() {}

int ParameterLinearizer::parameter_count() const { return pimpl->variable_count(); }

void ParameterLinearizer::set_traits( const engine::InputTraits& t )
    { pimpl->set_plane_count(t.plane_count()); }

void ParameterLinearizer::linearize( const engine::InputTraits& traits, gsl_vector* x )
{
    Eigen::VectorXd values = pimpl->linearize( traits );
    assert( values.rows() == int(x->size) );
    for (int i = 0; i < values.rows(); ++i)
        gsl_vector_set( x, i, values[i] );
}

void ParameterLinearizer::delinearize( const gsl_vector* x, engine::InputTraits& into ) {
    Eigen::VectorXd values( x->size );
    for (int i = 0; i < values.rows(); ++i)
        values[i] = gsl_vector_get( x, i );
    pimpl->delinearize( values, into );
}

Eigen::VectorXd ParameterLinearizer::Pimpl::linearize( const engine::InputTraits& traits ) const {
    assert( traits.plane_count() == int(planes.size()) );
    const int fluorophore = 0;
    for (int plane_index = 0; plane_index < traits.plane_count(); ++plane_index) {
        guf::PSF::Polynomial3D& m = planes[plane_index].get_expression();
        /* Set the parameters of the model to the traits' values. */
        guf::TraitValueFinder iv( fluorophore, traits.optics(plane_index) );
        boost::mpl::for_each< PSF::Variables >( 
            boost::bind( boost::ref(iv), _1, boost::ref(m) ) );
    }

    MultiPlane::Position parameters( multiplane->variable_count() );
    multiplane->get_position( parameters );
    return parameters;
}

void ParameterLinearizer::Pimpl::delinearize( const Eigen::VectorXd& parameters, engine::InputTraits& traits ) 
{
    const int fluorophore = 0;
    multiplane->set_position( parameters );
    for (int plane_index = 0; plane_index < traits.plane_count(); ++plane_index) {
        guf::PSF::Polynomial3D& m = planes[plane_index].get_expression();
        traits::Optics& o = traits.optics(plane_index);
        threed_info::Polynomial3D& p = boost::get<threed_info::Polynomial3D>(*o.depth_info());
        p.focal_planes() = m.get< guf::PSF::ZPosition >().cast< quantity<si::length> >();
        o.psf_size(fluorophore)->x() = quantity<si::length>(*m( guf::PSF::BestSigma<0>() ));
        o.psf_size(fluorophore)->y() = quantity<si::length>(*m( guf::PSF::BestSigma<1>() ));

        for (Direction dir = Direction_X; dir != Direction_2D; ++dir) {
            for (int term = polynomial_3d::FirstTerm; term <= polynomial_3d::LastTerm; ++term) {
                p.set_slope( dir, term, threed_info::Polynomial3D::WidthSlope(m.get_delta_sigma( dir, term )) );
            }
        }
    }
}

}
}
