#ifndef DSTORM_FITTER_MARQUARDTINFO_H
#define DSTORM_FITTER_MARQUARDTINFO_H

#include <fit++/FitFunction.hh>
#include <dStorm/engine/JobInfo_decl.h>

namespace dStorm {
namespace fitter {

class MarquardtConfig;

template <int VarC>
struct MarquardtInfo {
  public:
    fitpp::FitFunction<VarC> fit_function;
    MarquardtInfo( const MarquardtConfig& c, const engine::JobInfo& i );
};

}
}

#endif
