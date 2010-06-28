#ifndef LIBFITPP_EXPONENTIAL2D_PARAMETERHELPER_H
#define LIBFITPP_EXPONENTIAL2D_PARAMETERHELPER_H

#include <fit++/Exponential2D_impl.hh>

namespace fitpp {
namespace Exponential2D {

template <int Ks, int PM, int W, int H, bool Corr>
bool ParameterHelper<Ks,PM,W,H,Corr>::prepare(
    const typename Space::Variables& v,
    const typename Space::Constants& c,
    const int x_low, const int y_low
) {
    Base::extract( v, c );
    Base::template extract_param<SigmaX>( v, c, this->sx );
    Base::template extract_param<SigmaY>( v, c, this->sy );
    Base::template extract_param<SigmaXY>( v, c, this->rho );
    if ( Corr )
        this->ellip = (-rho.cwise().square()).cwise() + 1;

    if ( ! Base::check(x_low, y_low) ||
         ( rho.cwise() < -1.0  ).any() || ( rho.cwise() >  1.0  ).any() )
        return false;

    Base::precompute(x_low, y_low);
    return true;
};

}
}

#endif
