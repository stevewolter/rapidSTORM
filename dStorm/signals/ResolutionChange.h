#ifndef DSTORM_INPUT_RESOLUTIONCHANGED_H
#define DSTORM_INPUT_RESOLUTIONCHANGED_H

#include <boost/signals2/slot.hpp>
#include <boost/signals2/signal.hpp>
#include <dStorm/image/MetaInfo.h>

namespace dStorm {
namespace signals {

struct ResolutionChange 
: public boost::signals2::signal< void (const image::MetaInfo<2>::Resolutions&) > 
    {};

}
}

#endif
