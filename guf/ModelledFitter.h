#ifndef DSTORM_FITTER_GUF_NAIVEFITTER_H
#define DSTORM_FITTER_GUF_NAIVEFITTER_H

#include <Eigen/StdVector>
#include <boost/ptr_container/ptr_vector.hpp>

#include "engine/JobInfo_decl.h"
#include "fit_window/Plane.h"
#include "guf/Config_decl.h"
#include "guf/FunctionRepository.h"
#include "guf/MultiKernelModel.h"
#include "guf/NaiveFitter.h"
#include "Localization_decl.h"
#include "nonlinfit/AbstractFunction.h"
#include "nonlinfit/levmar/Fitter.h"
#include "nonlinfit/sum/AbstractFunction.h"
#include "nonlinfit/terminators/StepLimit.h"

namespace dStorm {
namespace guf {

/** Implementation of NaiveFitter for a concrete nonlinfit::Function. 
 *  This class is used by initializing its fit_position() to the start
 *  position and calling fit() to optimize the current position to an
 *  optimal value. */
template <class _Function>
class ModelledFitter
: public NaiveFitter
{
    typedef FunctionRepository<_Function> Repository;
    typedef boost::ptr_vector< Repository > PlaneFunctions;
    typedef nonlinfit::sum::AbstractFunction Function;

    Function fitter;
    PlaneFunctions evaluators;
    nonlinfit::levmar::Fitter lm;
    const nonlinfit::terminators::StepLimit step_limit;
    MultiKernelModelStack _model;

  public:
    ModelledFitter( const Config& config, const dStorm::engine::JobInfo& info );

    double fit( fit_window::PlaneStack& image, bool mle );

    MultiKernelModelStack& fit_position() { return _model; }
};

}
}

#endif
