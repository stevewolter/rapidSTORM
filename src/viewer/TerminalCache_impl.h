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
    size.keys.push_back( Display::KeyDeclaration("ADC", "total A/D counts per pixel", Colorizer::BrightnessDepth) );
}

template <typename Hueing>
TerminalCache<Hueing>::TerminalCache( 
    dStorm::Display::ResizeChange size
) : size(size)
{
}

template <typename Hueing>
void TerminalCache<Hueing>::setSize(
    const input::Traits< Image<int,2> >& traits
) {
    DEBUG("Setting size of image to " << traits.size.x() << " " <<traits.size.y());
    size.size = traits.size;
    size.pixel_size = *traits.resolution;
}

template <typename Hueing>
std::auto_ptr<dStorm::Display::Change> 
TerminalCache<Hueing>::get_result(const Colorizer& colorizer) const
{
    std::auto_ptr<dStorm::Display::Change> rv
        ( new dStorm::Display::Change(Hueing::KeyCount) );

    rv->do_resize = true;
    rv->resize_image = size;
    rv->do_change_image = true;
    rv->image_change.new_image = Im(size.size);

    if ( int(size.keys.size()) < Hueing::KeyCount ) {
        assert ( colorizer != NULL );
        for (int j = 1; j < Hueing::KeyCount; ++j ) {
            rv->resize_image.keys.push_back( colorizer.create_key_declaration( j ) );
        }
    }

    return rv;
}

}
}

#endif
