#ifndef DSTORM_VIEWER_LIVECACHE_IMPL_H
#define DSTORM_VIEWER_LIVECACHE_IMPL_H

#include "LiveCache_inline.h"
#include <dStorm/ImageTraits.h>

namespace dStorm {
namespace viewer {

template < typename Listener>
LiveCache<Listener>::LiveCache(HighDepth d) 
: pixels_by_value( d, HistogramPixel() ),
  in_depth(d)
{
}

template < typename Listener>
void LiveCache<Listener>::setSize( const input::Traits< Image<int,2> >& traits ) {
    pixels_by_position = Image<HistogramPixel,2>( traits.size );

    int w = traits.size.x() / cs_units::camera::pixel,
        h = traits.size.y() / cs_units::camera::pixel;
    for (int x = 0; x < w; x++)
      for (int y = 0; y < h; y++) {
        HistogramPixel& p = pixels_by_position(x,y);
        p.clear();
        p.x = x;
        p.y = y;
      }

    for (unsigned int i = 0; i < in_depth; i++) {
        pixels_by_value[i].clear();
    }

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
}

}
}

#endif
