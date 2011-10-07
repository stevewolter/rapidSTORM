#ifndef DSTORM_VIEWER_DISPLAY_INLINE_H
#define DSTORM_VIEWER_DISPLAY_INLINE_H

#include "Display.h"

namespace dStorm {
namespace viewer {

template <typename Colorizer>
void Display<Colorizer>::pixelChanged(int x, int y) {
    std::vector<bool>::reference is_on = ps[ y * ps_step + x ];
    if ( ! is_on ) {
        dStorm::Display::PixelChange*
            p = next_change->change_pixels.allocate();
        new(p) dStorm::Display::PixelChange(x,y);
        /* The color field will be set when the clean handler
            * runs. */
        is_on = true;
    }
}

template <typename Colorizer>
void Display<Colorizer>::notice_key_change( int index, 
        Pixel pixel, float value )
{
    dStorm::Display::KeyChange& k = 
        *next_change->changed_keys.front().allocate(1);
    k.index = index;
    k.color = pixel;
    k.value = value;
    next_change->changed_keys.front().commit(1);
}

}
}

#endif
