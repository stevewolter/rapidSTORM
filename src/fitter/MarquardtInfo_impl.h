#ifndef DSTORM_FITTER_MARQUARDTINFO_IMPL_H
#define DSTORM_FITTER_MARQUARDTINFO_IMPL_H

#include "MarquardtConfig.h"
namespace dStorm {
namespace fitter {

template <int VarC>
MarquardtInfo<VarC>::MarquardtInfo(
    const MarquardtConfig& c,
    const dStorm::engine::JobInfo&)
{
    fit_function.
        setStartLambda( c.marquardtStartLambda() );
    fit_function.
        setMaximumIterationSteps( c.maximumIterationSteps() );
    fit_function.
        setSuccessiveNegligibleStepLimit( 
            c.successiveNegligibleSteps() );

}

}
}

#endif
