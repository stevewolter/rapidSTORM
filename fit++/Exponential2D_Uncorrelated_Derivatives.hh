#ifndef EXPONENTIAL2D_UNCORRELATED_DERIVATIVES_HH
#define EXPONENTIAL2D_UNCORRELATED_DERIVATIVES_HH

#include <fit++/BlockReturner.hh>
#include <fit++/Exponential2D_impl.hh>
#include <fit++/Exponential2D_ParameterHelper.hh>
#include "Exponential_Uncorrelated_Derivatives.h"

namespace fitpp {
namespace Exponential2D {

template <class Special>
struct DerivativeHelper<Special,false>
: public Exponential::DerivativeHelper<Special,false>
{
    typedef typename Special::Space Space;
    typedef Exponential::DerivativeHelper<Special,false> Base;
    typedef typename Special::Data Data;

    inline bool prepare();

};

template <class Special>
bool 
DerivativeHelper<Special,false>::prepare() 
{
    if ( ! Base::prepare() )
        return false;
    if ( Space::template Parameter<SigmaX>::Variable ) {
        this->template getRow<SigmaX>()
            = - (this->prefactor.cwise() * this->sxI).asDiagonal() * ( this->xl.sqr.cwise() * this->xl.expTerm);
        this->template getColumn<SigmaX>()
            = this->yl.expTerm;
    }

    if ( Space::template Parameter<SigmaY>::Variable ) {
        this->template getRow<SigmaY>()
            = - this->prefactor.asDiagonal() * this->xl.expTerm;
        this->template getColumn<SigmaY>()
            = this->syI.asDiagonal() * ( this->yl.sqr.cwise() * this->yl.expTerm);
    }

    return true;
}

}
}

#endif
