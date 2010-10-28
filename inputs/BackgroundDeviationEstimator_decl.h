#ifndef DSTORM_INPUT_BACKGROUND_STDDEV_ESTIMATOR_DECL_H
#define DSTORM_INPUT_BACKGROUND_STDDEV_ESTIMATOR_DECL_H

#include <dStorm/input/chain/Filter_decl.h>
#include <memory>

namespace dStorm {
namespace BackgroundStddevEstimator {

class Config;
class Source;
class ChainLink;

std::auto_ptr<input::chain::Filter> makeLink();

}
}

#endif
