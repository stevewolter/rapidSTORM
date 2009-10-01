#ifndef DSTORM_GAUSSFITTER_H
#define DSTORM_GAUSSFITTER_H

#include "engine/SpotFitter.h"
#include "engine/Config.h"

namespace dStorm {
    /** The GaussFitter class provides a spot fitter implementation using
     *  a nonlinear fit with a or multiple Gaussian kernel(s).
     *
     *  The template parameters can be used to:
     *  - select fitting the standard deviation parameters
     *    (Free_Sigmas = true) or leaving them fixed 
     *    (Free_Sigmas = false),
     *  - perform quadrant residue analysis and, if necessary, 
     *    a 2-kernel fit. If Residue_Analysis is false, all relevant
     *    code will be disabled.
     *  - Select optimized code for the common case of negligible 
     *    correlation between X- and Y-axes. If Corr = false, the
     *    optimization is enabled and the correlation term ignored.
     **/
    template <bool Free_Sigmas, bool Residue_Analysis = false, bool Corr = Free_Sigmas>
    class GaussFitter;

    SpotFitter* select_gauss_fitter (const Config& config);
};

#endif
