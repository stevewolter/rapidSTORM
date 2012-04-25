#ifndef DSTORM_FITTER_GUF_NAIVEFITTER_H
#define DSTORM_FITTER_GUF_NAIVEFITTER_H

#include <Eigen/StdVector>
#include "NaiveFitter.h"
#include <dStorm/engine/JobInfo_decl.h>
#include "Config_decl.h"
#include <dStorm/Localization_decl.h>
#include <nonlinfit/sum/AbstractFunction.h>
#include "FitTerminator.h"
#include <nonlinfit/terminators/All.h>
#include <nonlinfit/terminators/StepLimit.h>
#include "FunctionRepository.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <nonlinfit/levmar/Fitter.h>
#include <nonlinfit/AbstractFunction.h>
#include "MultiKernelModel.h"

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
    typedef nonlinfit::sum::AbstractFunction< 
        typename Repository::result_type,
        typename Repository::Mover,
        nonlinfit::sum::BoundedPolicy<Config::maximum_plane_count>
    > Function;

    Function fitter;
    PlaneFunctions evaluators;
    nonlinfit::levmar::Fitter lm;
    typedef nonlinfit::terminators::Combined< 
            nonlinfit::terminators::StepLimit,
            FitTerminator<_Function>,
            std::logical_and<bool> >
        MyTerminator;
    const MyTerminator terminator;

    MultiKernelModelStack _model;

  public:
    ModelledFitter( const Config& config, const dStorm::engine::JobInfo& info );

    double fit( FittingRegionStack& image, bool mle );

    MultiKernelModelStack& fit_position() { return _model; }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
}

#endif
