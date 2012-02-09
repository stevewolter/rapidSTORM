#ifndef DSTORM_VIEWER_DISPLAY_INLINE_H
#define DSTORM_VIEWER_DISPLAY_INLINE_H

#include "Display.h"

namespace dStorm {
namespace viewer {

inline std::vector<bool>::reference
    BaseDisplay::is_on( const Im::Position& i )
{
    int offset = i[0].value();
    for (int j = 1; j < Im::Dim; ++j)
        offset += ps_step[j-1] * i[j].value();
    return ps[ offset ];
}

template <typename Colorizer>
void Display<Colorizer>::pixelChanged( const Im::Position& p ) {
    std::vector<bool>::reference is_on = this->is_on( p );
    if ( ! is_on ) {
        next_change->change_pixels.push_back( dStorm::display::PixelChange(p) );
        /* The color field will be set when the clean handler
            * runs. */
        is_on = true;
    }
}

template <typename Colorizer>
void Display<Colorizer>::notice_key_change( int index, 
        Pixel pixel, float value )
{
    next_change->changed_keys.front().push_back( dStorm::display::KeyChange(
        index, pixel, value ) );
}

}
}

#endif
