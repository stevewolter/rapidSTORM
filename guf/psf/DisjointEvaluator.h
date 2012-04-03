#ifndef DSTORM_GUF_PSF_DISJOINTEVALUATOR_H
#define DSTORM_GUF_PSF_DISJOINTEVALUATOR_H

#include "debug.h"
#include "BaseEvaluator.h"
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/include/at.hpp>
#include <nonlinfit/plane/fwd.h>
#include <nonlinfit/Evaluator.h>
#include "expressions.h"

namespace dStorm {
namespace guf {
namespace PSF {

using boost::fusion::at_c;
using boost::fusion::at;
using boost::fusion::vector;
using nonlinfit::DerivationSummand;
using nonlinfit::Xs;
using namespace boost::mpl;

template <typename Parameter, typename Dimension>
struct dimension_mismatch : public true_ {};
template <template <int Foo> class Param, int Dim>
struct dimension_mismatch< Param<Dim>, nonlinfit::Xs<Dim,LengthUnit> > 
    : public false_ {};
template <template <int Foo, int Bar> class Param, int Dim, int Term>
struct dimension_mismatch< Param<Dim,Term>, nonlinfit::Xs<Dim,LengthUnit> > 
    : public false_ {};
template <int Dim>
struct dimension_mismatch< nonlinfit::Xs<Dim,LengthUnit>, nonlinfit::Xs<Dim,LengthUnit> > 
    : public false_ {};

template <typename DerivationSummand>
struct is_always_zero {
    typedef boost::mpl::vector<MeanZ> Duals;
    typedef boost::mpl::vector<Amplitude,Prefactor> XOnly;
    typedef typename DerivationSummand::Parameter Parameter;
    typedef typename DerivationSummand::Summand Summand;
  public:
    typedef typename if_<
        contains< Duals, Parameter >,
        false_,
        typename if_<
            contains< XOnly, Parameter >,
            bool_< Summand::value == 1 >,
            dimension_mismatch< Parameter, Summand >
        >::type
    >::type type;
};

template <typename Num, typename Expression, int ChunkSize>
struct DisjointEvaluator
: public Parameters< Num, Expression >
{
    typedef Parameters< Num, Expression > Base;
    typedef Eigen::Array<Num, ChunkSize, 1> ChunkVector;
    typedef Eigen::Array<Num, 1, 1> SingleElement;
    typedef vector< ChunkVector, SingleElement >
        Precomputed;
    Precomputed normed, squared, expT;

    DisjointEvaluator() {}
    DisjointEvaluator( const Expression& e ) : Base(e) {}
    template <int Dim, int Size>
    void _precompute( const Eigen::Array<Num,Size,1>& v )
    {
        at_c<Dim>(normed) = (v - this->spatial_mean[Dim]) * this->sigmaI[Dim];
        at_c<Dim>(squared) = at_c<Dim>(normed).square();
        at_c<Dim>(expT) = (at_c<Dim>(squared) * -0.5).exp() * 
            ((Dim == 0) ? this->prefactor : 1.0);
    }

  public:
    template <typename DerivationSummand>
    struct is_always_zero {
        typedef typename PSF::is_always_zero<DerivationSummand>::type type;
    };
    template <typename Data>
    bool prepare_iteration( const Data& data ) {
        if ( !Base::prepare_iteration( data ) )
            return false;
        _precompute<0,ChunkSize>(data.xs); 
        return true;
    }

    void prepare_chunk( const Eigen::Array<Num,1,1>& y ) { 
        _precompute<1,1>(y); 
    }

    void value( Eigen::Array<Num,ChunkSize,1>& result ) 
      { result = at_c<0>(expT) * at_c<1>(expT)(0,0); }
    void add_value( Eigen::Array<Num,ChunkSize,1>& result ) 
      { result += at_c<0>(expT) * at_c<1>(expT)(0,0); }

    typedef XPosition OnlySummand;
    typedef YPosition OtherSummand;

    template <typename Target, class Dim>
    void multiply( Target target, Dim, Dim ) 
        { target *= at<Dim>(normed) * - this->sigmaI[Dim::value]; }
    template <typename Target, class Dim>
    void multiply( Target target, Mean<Dim::value>, Dim ) 
        { target *= at<Dim>(normed) * this->sigmaI[Dim::value]; }
    template <typename Target, class Dim>
    void multiply( Target target, MeanZ, Dim ) {
        target *= (at<Dim>(squared) - 1) * -this->z_deriv_prefactor[Dim::value];
    }
    template <typename Target, class Dim>
    void multiply( Target target, ZPosition<Dim::value>, Dim ) { 
        target *= (at<Dim>(squared) - 1) * this->z_deriv_prefactor[Dim::value];
    }
    template <typename Target, class Dim>
    void multiply( Target target, BestSigma<Dim::value>, Dim ) {
        target *= (at<Dim>(squared) - 1) * this->sigma_deriv[Dim::value];
    }
    template <typename Target>
    void multiply( Target target, Prefactor, OnlySummand ) 
        { target /= this->transmission; }
    template <typename Target>
    void multiply( Target target, Amplitude, OnlySummand ) 
        { target /= this->amplitude; }
    template <typename Target, class Dim, int Term>
    void multiply( Target target, DeltaSigma<Dim::value, Term>, Dim ) { 
        target *= this->delta_z_deriv_prefactor(Dim::value,Term) * (at<Dim>(squared) - 1);
    }
    template <typename Target, class Param, class Dim>
    void multiply( Target, Param, Dim ) {}

    template <typename Target, typename Summand, typename Parameter, class Dim>
    void derivative( Target target, DerivationSummand< Summand, Parameter, Dim > ) {
        target = at<Dim>(expT);
        if ( Summand::value == Dim::value ) {
            multiply( target.array(), Parameter(), Dim() );
        }
    }
};

}
}
}

namespace nonlinfit {

#define DSTORM_GUF_PSF_DISJOINT_SPECIALIZATION(Expression) \
template <typename Num, int ChunkSize> \
struct get_evaluator< \
    Expression, \
    plane::Disjoint<Num,ChunkSize,dStorm::guf::PSF::XPosition,\
                                  dStorm::guf::PSF::YPosition > \
> {\
    typedef dStorm::guf::PSF::DisjointEvaluator<Num, Expression, ChunkSize > type; \
};
DSTORM_GUF_PSF_DISJOINT_SPECIALIZATION(dStorm::guf::PSF::Polynomial3D)
DSTORM_GUF_PSF_DISJOINT_SPECIALIZATION(dStorm::guf::PSF::No3D)
DSTORM_GUF_PSF_DISJOINT_SPECIALIZATION(dStorm::guf::PSF::Spline3D)
#undef DSTORM_GUF_PSF_DISJOINT_SPECIALIZATION

}


#endif
