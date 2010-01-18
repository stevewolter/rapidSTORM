#ifndef DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_INLINE_H
#define DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_INLINE_H

#include "ImageDiscretizer.h"

namespace dStorm {
namespace viewer {
namespace DiscretizedImage {

template <typename Colorizer, typename ImageListener>
const typename ImageDiscretizer<Colorizer, ImageListener>::HighDepth
ImageDiscretizer<Colorizer, ImageListener>::background_threshold = 1;

template <typename Colorizer, typename ImageListener>
float 
ImageDiscretizer<Colorizer, ImageListener>::key_value( LowDepth key )
{
    int n = -1; 
    while ( transition[n+1] <= key ) n++;
    return (n+0.5f) / disc_factor;
}

template <typename Colorizer, typename ImageListener>
void ImageDiscretizer<Colorizer, ImageListener>
::change( int x, int y, HighDepth to )
{
    this->publish().pixelChanged( x, y );

    pixels_by_value[to]
        .push_back( pixels_by_position(x,y) );
}

template <typename Colorizer, typename ImageListener>
void ImageDiscretizer<Colorizer, ImageListener>
::updatePixel(int x, int y, float from, float to) 
{
    colorizer.updatePixel( x, y, from, to );

    if ( to > max_value_used_for_disc_factor ) {
        if ( from <= max_value_used_for_disc_factor )
            ++pixels_above_used_max_value;
        max_value = std::max( to, max_value );
    }

    HighDepth o = discretize( from ),
              n = discretize( to );
    
    if ( o != n ) {
        ++histogram[ n ];
        if ( histogram[o] > 0U )
            --histogram[o];
        change( x, y, n );
    }
}

template <typename Colorizer, typename ImageListener>
inline unsigned long int ImageDiscretizer<Colorizer, ImageListener>::non_background_pixels()
{
    long int accum = 0;
    for (unsigned int i = 0; i < background_threshold; i++)
        accum += histogram[i];
    return pixels_by_position.size() - accum;
}

}
}
}

#endif
