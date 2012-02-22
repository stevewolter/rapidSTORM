#ifndef DSTORM_VIEWER_LIVECACHE_H
#define DSTORM_VIEWER_LIVECACHE_H

#include <vector>
#include "Publisher.h"
#include "HighDepth.h"
#include <dStorm/Pixel.h>
#include "Image.h"

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

}
}

#endif
