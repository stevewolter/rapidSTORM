#ifndef DSTORM_INPUT_MEDIANFILTER_H
#define DSTORM_INPUT_MEDIANFILTER_H

#include <dStorm/input/fwd.h>
#include <memory>

namespace dStorm {
namespace MedianFilter {

std::auto_ptr<input::Link> make_link();

}
}

#endif
