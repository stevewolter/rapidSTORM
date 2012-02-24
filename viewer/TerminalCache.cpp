#include "TerminalCache.h"
#include "debug.h"
#include <boost/units/io.hpp>
#include "Display.h"

namespace dStorm {
namespace viewer {

TerminalCache::TerminalCache()
{
}

void TerminalCache::setSize( 
    const display::ResizeChange& size
)
{
    this->size = size;
}

void TerminalCache::setSize(
    const Im::MetaInfo& traits
) {
    DEBUG("Setting size of image to " << traits.size.x() << " " <<traits.size.y());
    size.set_size( Im::Size(traits.size) );
    const int display_dim = display::Image::Dim;
    for (int i = 0; i < std::min(display_dim, Im::Dim); ++i)
        if ( traits.has_resolution(i) )
            size.pixel_sizes[i] = traits.resolution(i);
        else
            size.pixel_sizes[i].value = -1 / camera::pixel;
}

}
}
