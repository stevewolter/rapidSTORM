#ifndef DSTORM_TRANSMISSIONS_COLOURDISPLAY_BASE_H
#define DSTORM_TRANSMISSIONS_COLOURDISPLAY_BASE_H

#include <limits>
#include <dStorm/Pixel.h>
#include <stdint.h>
#include <boost/array.hpp>

namespace dStorm {
namespace viewer {
namespace colour_schemes {

typedef boost::array<float,3> RGBWeight;

inline dStorm::Pixel operator*( const RGBWeight& r, uint8_t brightness );
inline dStorm::Pixel operator*( uint8_t brightness, const RGBWeight& r );

void rgb_weights_from_hue_saturation
    ( float hue, float saturation, RGBWeight& weight );
void convert_xy_tone_to_hue_sat( 
    float x, float y, float& hue, float& sat );

inline dStorm::Pixel operator*( const RGBWeight& r, uint8_t b ) {
    for (int i = 0; i < 2; ++i) {
        assert ( int(round(r[i] * b )) >= std::numeric_limits<uint8_t>::min() );
        assert ( int(round(r[i] * b )) <= std::numeric_limits<uint8_t>::max() );
    }
    return dStorm::Pixel( round(r[0] * b), round(r[1] * b), round(r[2] * b) );
}
inline dStorm::Pixel operator*( uint8_t b, const RGBWeight& r ) {
    for (int i = 0; i < 2; ++i) {
        assert ( int(round(r[i] * b )) >= std::numeric_limits<uint8_t>::min() );
        assert ( int(round(r[i] * b )) <= std::numeric_limits<uint8_t>::max() );
    }
    return dStorm::Pixel( round(r[0] * b), round(r[1] * b), round(r[2] * b) );
}

}
}
}

#endif
