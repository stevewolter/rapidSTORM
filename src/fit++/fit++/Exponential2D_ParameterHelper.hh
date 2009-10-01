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
) throw() {
    const int Width = ((W==Eigen::Dynamic)?width:W);
    const int Height = ((H==Eigen::Dynamic)?height:H);

    extract_param<MeanX>( v, c, x0 );
    extract_param<MeanY>( v, c, y0 );
    extract_param<SigmaX>( v, c, sx );
    extract_param<SigmaY>( v, c, sy );
    extract_param<SigmaXY>( v, c, rho );
    extract_param<Amp>( v, c, amp );

    if (
        /* Center out of lattice? */
            ( x0.cwise() < x_low ).any() 
        ||( x0.cwise() > (x_low + Width-1) ).any()
        ||( y0.cwise() < y_low ).any()
        ||( y0.cwise() > (y_low + Height-1) ).any()
        /* Sigmas insane? */
        ||( rho.cwise() < -1.0  ).any() || ( rho.cwise() >  1.0  ).any()
        ||( sx.cwise() < 0 ).any() || ( sy.cwise() < 0 ).any()
        /* Amplitude insane? */
        ||( amp.cwise() < 0 ).any() )
        return false;

    if ( Corr )
        ellip = (-rho.cwise().square()).cwise() + 1;
    sxI = sx.cwise().inverse();
    syI = sy.cwise().inverse();
    norms = (2 * M_PI * (sx.cwise() * sy)).cwise().inverse();
    if ( Corr )
        norms.cwise() /= (ellip.cwise().sqrt());
    prefactor = norms.cwise() * amp;
    ellipI = ellip.cwise().inverse();

    shift = Space::template ParamIndex<Shift,0>::value(v, c);
    this->x_low = x_low;

    xl.prepare( x_low, x0, sxI );
    yl.prepare( y_low, y0, syI );
    return true;
};

}
}

#endif
