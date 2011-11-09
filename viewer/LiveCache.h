#ifndef DSTORM_VIEWER_LIVECACHE_H
#define DSTORM_VIEWER_LIVECACHE_H

#include <vector>
#include "Publisher.h"
#include "HighDepth.h"
#include <dStorm/Image.h>
#include <dStorm/Pixel.h>

namespace dStorm {
namespace viewer {

struct HistogramPixel { 
    /** Pixel position in image */
    unsigned short x, y;
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
    std::vector<HistogramPixel> pixels_by_value;
    Image<HistogramPixel,2> pixels_by_position;
    HighDepth in_depth;

    void set_xy();
    static bool list_is_loop_free( HistogramPixel* start );

  public:
    static const int NeedLiveHistogram = true;
    typedef typename Listener::Colorizer Colorizer;
    LiveCache(HighDepth depth);
    LiveCache(HighDepth depth, Image<HistogramPixel,2>::Size size);

    void setSize( const input::Traits< Image<int,2> >& );
    inline void pixelChanged( int x, int y, HighDepth to );
    inline void changeBrightness( HighDepth at );
    void clean( bool final );
    inline void notice_key_change( int index, Pixel pixel, float value );
    void clear();
};

}
}

#endif
