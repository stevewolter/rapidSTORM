#ifndef DSTORM_INPUT_ROIFILTER_H
#define DSTORM_INPUT_ROIFILTER_H

#include <dStorm/input/fwd.h>
#include <memory>

namespace dStorm {
namespace ROIFilter {

std::auto_ptr<input::Link> make_link();

}
}

#endif
