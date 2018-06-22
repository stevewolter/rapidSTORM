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
#include <nonlinfit/sum/AbstractFunction.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include "engine/JobInfo.h"
#include "engine/InputTraits.h"
#include "fit_window/Stack.h"
#include <nonlinfit/AbstractTerminator.h>

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
  terminator( 
    nonlinfit::terminators::StepLimit(config.maximumIterationSteps()),
    FitTerminator<_Function>(config) )
{
    for (int i = 0; i < info.traits.plane_count(); ++i ) {
        evaluators.push_back( new Repository() );
        _model.push_back( MultiKernelModel( evaluators[i].get_expression() ) );
    }
}

template <class _Function>
double ModelledFitter<_Function>::fit( 
    fit_window::Stack& data,
    bool mle
) {
    typedef nonlinfit::AbstractTerminatorAdaptor< MyTerminator, typename Function::Position >
        AbstractTerminator;
    typedef nonlinfit::AbstractTerminator< typename Function::Position >
        TerminatorInterface;
    typedef nonlinfit::AbstractFunction<double> AbstractFunction;
    typedef nonlinfit::AbstractMoveable<double> AbstractMoveable;
    for ( typename fit_window::Stack::iterator b = data.begin(), i = b, e = data.end(); i != e; ++i )
        fitter.set_fitter( i-b, *evaluators[i-b]( *i, mle ), evaluators[i-b].get_moveable() );

    MyTerminator concrete_terminator(terminator);
    AbstractTerminator abstract_terminator(concrete_terminator);
    return lm.fit< 
        AbstractFunction,
        AbstractMoveable,
        TerminatorInterface& >
        ( fitter, fitter, abstract_terminator );
}

}
}
#endif
