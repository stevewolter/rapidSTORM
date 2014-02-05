#include "viewer/LiveCache.h"
#include "viewer/Display.h"
#include "viewer/ColourScheme.h"
#include "viewer/Display.h"
#include <dStorm/image/constructors.h>
#include <dStorm/image/MetaInfo.h>
#include <dStorm/image/iterator.h>

namespace dStorm {
namespace viewer {

LiveCache::LiveCache(HighDepth d) 
: pixels_by_value( d, HistogramPixel() ),
  in_depth(d)
{
}

LiveCache::LiveCache(
    HighDepth d, Image<HistogramPixel,Im::Dim>::Size size) 
: pixels_by_value( d, HistogramPixel() ),
  pixels_by_position( size ),
  in_depth(d)
{
}

void LiveCache::setSize( const Im::MetaInfo& traits ) {
    pixels_by_position = HistogramImage( traits.size );

    this->publish().setSize( traits );
}

void LiveCache::clean( bool final ) {
    this->publish().clean(final);
}

void LiveCache::clear() {
    for (unsigned int i = 0; i < in_depth; i++)
        pixels_by_value[i].unlink();
    this->publish().clear();
}

}
}
