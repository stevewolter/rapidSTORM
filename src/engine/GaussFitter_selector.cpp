#include "engine/GaussFitter_impl.h"

namespace dStorm {
namespace engine {

template <int Mask> 
SpotFitter* instantiate_Gauss_Fitter(int mask, const Config& c) {
    if ( mask == Mask )
        return new GaussFitter< Mask & 0x1, Mask & 0x2, Mask & 0x5 >(c);
    else if ( Mask > 0 )
        return instantiate_Gauss_Fitter<(Mask==0)?0:Mask-1>(mask, c);
    else
        throw std::runtime_error("Recursion base missed "
                                 "in select_gauss_fitter.");
}

SpotFitter* select_gauss_fitter (const Config& c) {
    bool residue_analysis = ( c.asymmetry_threshold() <= 0.99 );
    bool free_sigmas = c.freeSigmaFitting();
    bool correlation = free_sigmas || 
        c.sigma_xy_negligible_limit() <= abs( c.sigma_xy() );

    int mask = (residue_analysis ? 0x2 : 0) + 
               (free_sigmas ? 0x1 : 0) +
               (correlation ? 0x4 : 0);
    return instantiate_Gauss_Fitter<7>(mask, c);
}

}
}
