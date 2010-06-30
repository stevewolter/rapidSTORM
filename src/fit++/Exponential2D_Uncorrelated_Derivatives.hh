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
        this->template getRow<SigmaX>().transpose()
            = (this->prefactor.cwise() * this->sxI).asDiagonal() * ( (this->xl.sqr.cwise() - 1).cwise() * this->xl.expTerm);
        this->template getColumn<SigmaX>().transpose()
            = this->yl.expTerm;
    }

    if ( Space::template Parameter<SigmaY>::Variable ) {
        this->template getRow<SigmaY>().transpose()
            = this->prefactor.asDiagonal() * this->xl.expTerm;
        this->template getColumn<SigmaY>().transpose()
            = this->syI.asDiagonal() * ( (this->yl.sqr.cwise() - 1).cwise() * this->yl.expTerm);
    }

    DEBUG("Successfully prepared");
    return true;
}

}
}

#endif
