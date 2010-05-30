#ifndef DSTORM_TERMINALCACHE_IMPL_H
#define DSTORM_TERMINALCACHE_IMPL_H

#include "TerminalCache.h"
#include "debug.h"
#include <boost/units/io.hpp>

namespace dStorm {
namespace viewer {

template <typename Hueing>
void TerminalCache<Hueing>::setSize(
    const input::Traits< Image<int,2> >& traits
) {
    DEBUG("Setting size of image to " << traits.size.x() << " " <<traits.size.y());
    size.size = traits.size;
    size.key_size = Colorizer::BrightnessDepth;
    size.pixel_size = *traits.resolution;
}

template <typename Hueing>
std::auto_ptr<dStorm::Display::Change> 
TerminalCache<Hueing>::get_result()
{
    std::auto_ptr<dStorm::Display::Change> rv
        ( new dStorm::Display::Change() );

    rv->do_resize = true;
    rv->resize_image = size;
    rv->do_change_image = true;
    rv->image_change.new_image = Im(size.size);

    return rv;
}

}
}

#endif
