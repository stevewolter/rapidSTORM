#ifndef DSTORM_TERMINALCACHE_IMPL_H
#define DSTORM_TERMINALCACHE_IMPL_H

#include "TerminalCache.h"
#include "debug.h"
#include <boost/units/io.hpp>

namespace dStorm {
namespace viewer {

template <typename Hueing>
TerminalCache<Hueing>::TerminalCache()
{
    size.keys.push_back( display::KeyDeclaration("ADC", "total A/D counts per pixel", Colorizer::BrightnessDepth) );
}

template <typename Hueing>
void TerminalCache<Hueing>::setSize( 
    const display::ResizeChange& size
)
{
    this->size = size;
}

template <typename Hueing>
void TerminalCache<Hueing>::setSize(
    const input::Traits< Im >& traits
) {
    DEBUG("Setting size of image to " << traits.size.x() << " " <<traits.size.y());
    size.set_size( Im::Size(traits.size) );
    for (int i = 0; i < std::min(2, Im::Dim); ++i)
        if ( traits.plane(0).resolution(i).is_initialized() )
            size.pixel_sizes[i] = *traits.plane(0).resolution(i);
        else
            size.pixel_sizes[i].value = -1 / camera::pixel;
}

template <typename Hueing>
std::auto_ptr<display::Change> 
TerminalCache<Hueing>::get_result(const Colorizer& colorizer) const
{
    std::auto_ptr<display::Change> rv
        ( new display::Change(Hueing::KeyCount) );

    rv->do_resize = true;
    rv->resize_image = size;
    rv->do_change_image = true;
    rv->image_change.new_image = display::Image(size.size);

    if ( int(size.keys.size()) < Hueing::KeyCount ) {
        for (int j = 1; j < Hueing::KeyCount; ++j ) {
            rv->resize_image.keys.push_back( colorizer.create_key_declaration( j ) );
        }
    }

    return rv;
}

}
}

#endif
