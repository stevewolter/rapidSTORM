#ifndef DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_INLINE_H
#define DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_INLINE_H

#include "ImageDiscretizer.h"

namespace dStorm {
namespace viewer {

template < typename ImageListener>
const HighDepth
Discretizer< ImageListener>::background_threshold = 1;

template < typename ImageListener>
float 
Discretizer< ImageListener>::key_value( LowDepth key )
{
    int n = -1; 
    while ( transition[n+1] <= key ) n++;
    return (n+0.5f) / disc_factor;
}

template < typename ImageListener>
void Discretizer< ImageListener>
::updatePixel(int x, int y, float from, float to) 
{
    colorizer.updatePixel( x, y, from, to );

    if ( ImageListener::NeedLiveHistogram ) {
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
            this->publish().pixelChanged( x, y, n );
        }
    } else {
        max_value = std::max( max_value, to );
    }
}

template < typename ImageListener>
inline unsigned long int Discretizer< ImageListener>::non_background_pixels()
{
    long int accum = 0;
    for (unsigned int i = 0; i < background_threshold; i++)
        accum += histogram[i];
    return binned_image.size_in_pixels() - accum;
}

}
}

#endif