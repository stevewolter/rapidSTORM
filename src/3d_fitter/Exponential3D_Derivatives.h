#ifndef EXPONENTIAL3D_UNCORRELATED_DERIVATIVES_HH
#define EXPONENTIAL3D_UNCORRELATED_DERIVATIVES_HH

#include <fit++/BlockReturner.hh>
#include "Exponential3D_impl.hh"
#include "Exponential3D_ParameterHelper.hh"
#include <fit++/Exponential_Uncorrelated_Derivatives.h>

namespace fitpp {
namespace Exponential3D {

template <class Special>
struct DerivativeHelper
: public Exponential::DerivativeHelper<Special,false>
{
    typedef typename Special::Space Space;
    typedef Exponential::DerivativeHelper<Special,false> Base;
    typedef typename Special::Data Data;

    static const int H = Super::Height, W = Super::Width;
    Eigen::Matrix<double,W,H> z_sum;

    inline bool prepare();
    inline void compute(
        const Data& data,
        Data& residues,
        typename Space::Vector& gradient,
        typename Space::Matrix& hessian
    ) const;
};

template <class Special>
bool 
DerivativeHelper<Special>::prepare() 
{
    this->template getRow<MeanZ>() = 1;
    this->template getColumn<MeanZ>() = 1;

    return Base::prepare();
}
{


}
}

#endif
