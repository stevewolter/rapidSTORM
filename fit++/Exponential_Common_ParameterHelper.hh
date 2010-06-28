#ifndef LIBFITPP_EXPONENTIAL_PARAMETERHELPER_H
#define LIBFITPP_EXPONENTIAL_PARAMETERHELPER_H

#include "Exponential_Common.h"

namespace fitpp {
namespace Exponential {

template <typename Space, int W, int H, bool Corr>
bool ParameterHelper<Space,W,H,Corr>::check(
    const int x_low, const int y_low
) {
    const int Width = ((W==Eigen::Dynamic)?width:W);
    const int Height = ((H==Eigen::Dynamic)?height:H);

    if (
        /* Center out of lattice? */
            ( x0.cwise() < x_low ).any() 
        ||( x0.cwise() > (x_low + Width-1) ).any()
        ||( y0.cwise() < y_low ).any()
        ||( y0.cwise() > (y_low + Height-1) ).any()
        /* Sigmas insane? */
        ||( sx.cwise() < 0 ).any() || ( sy.cwise() < 0 ).any()
        /* Amplitude insane? */
        ||( amp.cwise() < 0 ).any() )
        return false;
    else
        return true;
}

template <typename Space, int W, int H, bool Corr>
void ParameterHelper<Space,W,H,Corr>::precompute(
    const int x_low, const int y_low
) {
    sxI = sx.cwise().inverse();
    syI = sy.cwise().inverse();
    norms = (2 * M_PI * (sx.cwise() * sy)).cwise().inverse();
    if ( Corr ) {
        norms.cwise() /= (ellip.cwise().sqrt());
        ellipI = ellip.cwise().inverse();
    }
    prefactor = norms.cwise() * amp;

    xl.prepare( x_low, x0, sxI );
    yl.prepare( y_low, y0, syI );
};

}
}

#endif
