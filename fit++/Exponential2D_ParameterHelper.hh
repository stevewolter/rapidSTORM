#ifndef LIBFITPP_EXPONENTIAL2D_PARAMETERHELPER_H
#define LIBFITPP_EXPONENTIAL2D_PARAMETERHELPER_H

#include <fit++/Exponential2D_impl.hh>

namespace fitpp {
namespace Exponential2D {

template <int Ks, int PM, int W, int H, bool Corr>
bool ParameterHelper<Ks,PM,W,H,Corr>::prepare(
    const typename Space::Variables& v,
    const typename Space::Constants& c,
    const int x_low, const int y_low, const int
) {
    Base::extract( v, c );
    Base::template extract_param<SigmaX>( v, c, this->sx );
    Base::template extract_param<SigmaY>( v, c, this->sy );
    if ( Corr ) {
        Base::template extract_param<SigmaXY>( v, c, this->rho );
        this->ellip = (-rho.cwise().square()).cwise() + 1;
    }

    if ( ! Base::check(x_low, y_low) ||
         ( Corr && ( rho.cwise().abs().cwise() > 1.0  ).any() ) )
    {
        DEBUG("Check failed");
        return false;
    }

    DEBUG("Check passed");
    Base::precompute(x_low, y_low);
    return true;
};

}
}

#endif
