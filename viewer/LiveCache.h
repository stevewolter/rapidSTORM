#ifndef DSTORM_VIEWER_LIVECACHE_H
#define DSTORM_VIEWER_LIVECACHE_H

#include <vector>
#include "Publisher.h"
#include "HighDepth.h"
#include <dStorm/Pixel.h>
#include "Image.h"
#include <dStorm/image/find_by_offset.hpp>

namespace dStorm {
namespace viewer {

struct HistogramPixel { 
    /** Linked list with pixels of same value. */
    HistogramPixel *prev, *next; 

    HistogramPixel() { clear(); }
    HistogramPixel(const HistogramPixel&) { clear(); }
    inline void push_back(HistogramPixel& node);
    inline void unlink();
    void clear() { prev = next = this; }
    bool empty() { return ( next == this ); }
};

template <typename Listener>
class LiveCache :
    public Publisher<Listener>
{
    typedef typename Listener::Colorizer Colorizer;
    std::vector<HistogramPixel> pixels_by_value;
    typedef Image<HistogramPixel,Im::Dim> HistogramImage;
    HistogramImage pixels_by_position;
    HighDepth in_depth;

  public:
    static const int NeedLiveHistogram = true;
    LiveCache(HighDepth depth);
    LiveCache(HighDepth depth, Im::Size size);

    void setSize( const image::MetaInfo<Im::Dim>& );
    inline void pixelChanged( const Im::Position& p, HighDepth to );
    inline void changeBrightness( HighDepth at );
    void clean( bool final );
    inline void notice_key_change( int index, Pixel pixel, float value );
    void clear();
};

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
void LiveCache<Listener>::pixelChanged( const Im::Position& p, HighDepth to ) {
    pixels_by_value[to]
        .push_back( pixels_by_position(p) );

    this->publish().pixelChanged( p );
}

template < typename Listener>
void LiveCache<Listener>::changeBrightness( HighDepth i ) {
    for ( HistogramPixel* j = pixels_by_value[i].next; 
                    j != &pixels_by_value[i]; j = j->next)
        this->publish().pixelChanged( find_by_offset( pixels_by_position,
            j - pixels_by_position.ptr() ) );
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
