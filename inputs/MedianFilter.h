#ifndef DSTORM_INPUT_MEDIANFILTER_H
#define DSTORM_INPUT_MEDIANFILTER_H

#include <dStorm/input/fwd.h>
#include <memory>

class TestState;

namespace dStorm {
namespace median_filter {

std::auto_ptr<input::Link> make_link();
void unit_test( TestState& );

}
}

#endif
