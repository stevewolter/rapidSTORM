#ifndef DSTORM_FITTER_GUF_NAIVEFITTER_INTERFACE_H
#define DSTORM_FITTER_GUF_NAIVEFITTER_INTERFACE_H

#include <memory>
#include <vector>

#include "guf/Config.h"
#include "guf/FitFunctionFactory.h"
#include "guf/MultiKernelModel.h"
#include "engine/JobInfo.h"
#include "fit_window/Plane.h"
#include "nonlinfit/levmar/Fitter.h"
#include "nonlinfit/sum/AbstractFunction.h"
#include "nonlinfit/terminators/StepLimit.h"

namespace dStorm {
namespace guf {

class MultiKernelModelStack;

class NaiveFitter {
  public:
    NaiveFitter(const Config&, const dStorm::engine::JobInfo&, int kernel_count);

    MultiKernelModelStack& fit_position() { return model_stack; }

    /** Optimize the current state set by fit_position() using 
     *  Levenberg-Marquardt minimization. If highest_residue is not null, store
     *  the location of the highest residue there.
     *
     *  \returns The new function value, which is the sum of squared residues
     *           for mle == false and the negative likelihood for mle == true.
     **/
    double fit( fit_window::PlaneStack& data, bool mle, Spot* highest_residue, double* r_value );

  private:
    std::vector<std::unique_ptr<FitFunctionFactory>> function_creators;
    nonlinfit::sum::VariableMap variable_map;
    boost::optional<nonlinfit::sum::AbstractFunction> plane_combiner;
    nonlinfit::levmar::Fitter lm;
    const nonlinfit::terminators::StepLimit step_limit;
    MultiKernelModelStack model_stack;
};

}
}

#endif
