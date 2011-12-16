#ifndef DSTORM_INPUT_RESOLUTIONCHANGED_H
#define DSTORM_INPUT_RESOLUTIONCHANGED_H

#include <string>
#include <boost/signals2/slot.hpp>
#include <boost/signals2/signal.hpp>
#include <dStorm/traits/optics.h>

namespace dStorm {
namespace input {

struct ResolutionChange 
: public boost::signals2::signal< void (const traits::Optics<2>::Resolutions&) > 
    {};

}
}

#endif
