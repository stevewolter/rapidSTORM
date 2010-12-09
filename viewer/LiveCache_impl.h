#ifndef DSTORM_VIEWER_LIVECACHE_IMPL_H
#define DSTORM_VIEWER_LIVECACHE_IMPL_H

#include "LiveCache_inline.h"
#include <dStorm/ImageTraits.h>
#include <dStorm/image/iterator.h>

namespace dStorm {
namespace viewer {

template < typename Listener>
LiveCache<Listener>::LiveCache(HighDepth d) 
: pixels_by_value( d, HistogramPixel() ),
  in_depth(d)
{
}

template < typename Listener>
LiveCache<Listener>::LiveCache(
    HighDepth d, Image<HistogramPixel,2>::Size size) 
: pixels_by_value( d, HistogramPixel() ),
  pixels_by_position( size ),
  in_depth(d)
{
    set_xy();
}

template < typename Listener>
void LiveCache<Listener>::set_xy() {
    Image<HistogramPixel,2>::iterator i;
    for ( i = pixels_by_position.begin(); i != pixels_by_position.end(); ++i)
    {
        i->x = i.x();
        i->y = i.y();
    }
}

template < typename Listener>
void LiveCache<Listener>::setSize( const input::Traits< Image<int,2> >& traits ) {
    pixels_by_position = Image<HistogramPixel,2>( traits.size );

    set_xy();

    this->publish().setSize( traits );
}

template < typename Listener>
void LiveCache<Listener>::clean( bool final ) {
    this->publish().clean(final);
}

template < typename Listener>
void LiveCache<Listener>::clear() {
    for (unsigned int i = 0; i < in_depth; i++)
        pixels_by_value[i].unlink();
    this->publish().clear();
}

}
}

#endif
