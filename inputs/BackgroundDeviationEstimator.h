#ifndef DSTORM_INPUT_BACKGROUND_STDDEV_ESTIMATOR_H
#define DSTORM_INPUT_BACKGROUND_STDDEV_ESTIMATOR_H

#include <dStorm/input/fwd.h>
#include <memory>

namespace dStorm {
namespace BackgroundStddevEstimator {

std::auto_ptr<input::Link> makeLink();

}
}

#endif
