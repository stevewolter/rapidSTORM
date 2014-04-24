#ifndef DSTORM_FITTER_GUF_NAIVEFITTER_IMPL_H
#define DSTORM_FITTER_GUF_NAIVEFITTER_IMPL_H

#include <Eigen/StdVector>
#include <nonlinfit/TermParameter.h>

#include "guf/Config.h"

#undef DEBUG

#include "guf/ModelledFitter.h"
#include "guf/MultiKernelModel.hpp"
#include "gaussian_psf/is_plane_dependent.h"
#include <nonlinfit/make_bitset.h>
#include <nonlinfit/sum/VariableMap.h>
#include <nonlinfit/AbstractFunction.h>
#include <nonlinfit/sum/AbstractFunction.h>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include "engine/JobInfo.h"
#include "engine/InputTraits.h"
#include "fit_window/Plane.h"
#include "LengthUnit.h"

#include "debug.h"

namespace dStorm {
namespace guf {

template <class _Function>
ModelledFitter<_Function>::ModelledFitter(
    const Config& config, 
    const dStorm::engine::JobInfo& info)
: fitter( 
     nonlinfit::sum::VariableMap( 
        info.traits.plane_count(),
        make_bitset( 
            typename _Function::Variables(), 
            gaussian_psf::is_plane_independent( 
                config.laempi_fit(), 
                config.disjoint_amplitudes() ) ) ) 
  ),
  lm(config.make_levmar_config()),
  step_limit(config.maximumIterationSteps())
{
    // TODO: Set the terminating step length.
    for (int i = 0; i < info.traits.plane_count(); ++i ) {
        evaluators.push_back( new Repository(config) );
        _model.push_back( MultiKernelModel( evaluators[i].get_expression() ) );
        for (gaussian_psf::BaseExpression& gaussian : _model.back()) {
            gaussian.set_negligible_step_length(ToLengthUnit(config.negligible_x_step()));
            gaussian.set_relative_epsilon(config.relative_epsilon());
        }
        _model.back().background_model().set_relative_epsilon(config.relative_epsilon());
    }
}

template <class _Function>
double ModelledFitter<_Function>::fit( 
    fit_window::PlaneStack& data,
    bool mle
) {
    typedef nonlinfit::AbstractFunction<double> AbstractFunction;
    std::vector<std::unique_ptr<nonlinfit::AbstractFunction<double>>> functions;
    for ( typename fit_window::PlaneStack::iterator b = data.begin(), i = b, e = data.end(); i != e; ++i ) {
        functions.push_back(evaluators[i-b].create_function(*i, mle));
        fitter.set_fitter( i-b, *functions.back() );
    }

    nonlinfit::terminators::StepLimit step_limit(this->step_limit);
    return lm.fit( fitter, step_limit );
}

}
}
#endif
