#ifndef DSTORM_VIEWER_LIVECACHE_INLINE_H
#define DSTORM_VIEWER_LIVECACHE_INLINE_H

#include "LiveCache.h"

namespace dStorm {
namespace viewer {

void HistogramPixel::push_back(HistogramPixel& node)
{
    node.prev->next = node.next;
    node.next->prev = node.prev;
    node.prev = prev;
    node.next = this;
    prev->next = &node;
    prev = &node;
}

void HistogramPixel::unlink() {
    prev->next = next;
    next->prev = prev;
    clear();
}

template < typename Listener>
void LiveCache<Listener>::pixelChanged( int x, int y, HighDepth to ) {
    pixels_by_value[to]
        .push_back( pixels_by_position(x,y) );

    this->publish().pixelChanged( x, y );
}

template < typename Listener>
void LiveCache<Listener>::changeBrightness( HighDepth i ) {
    assert( list_is_loop_free( &pixels_by_value[i] ) );
    for ( HistogramPixel* j = pixels_by_value[i].next; 
                    j != &pixels_by_value[i]; j = j->next)
        this->publish().pixelChanged( j->x, j->y );
}

template <typename Listener>
void LiveCache<Listener>::notice_key_change(
    int index, Pixel pixel, float value )
{
    this->publish().notice_key_change( index, pixel, value );
}

}
}

#endif
