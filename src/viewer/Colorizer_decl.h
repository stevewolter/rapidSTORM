#ifndef DSTORM_VIEWER_COLORIZER_DECL_H
#define DSTORM_VIEWER_COLORIZER_DECL_H

#include <stdint.h>
#include <boost/array.hpp>
#include <dStorm/Pixel_decl.h>

namespace dStorm {
namespace viewer {

namespace ColourSchemes {
    enum Scheme { BlackWhite, BlackRedYellowWhite,
                  FixedHue, TimeHue, ExtraHue, ExtraSaturation,
                  FirstColourModel = BlackWhite,
                  LastColourModel = ExtraSaturation};

    typedef boost::array<float,3> RGBWeight;

    void rgb_weights_from_hue_saturation
        ( float hue, float saturation, RGBWeight& weight );
    void convert_xy_tone_to_hue_sat( 
        float x, float y, float& hue, float& sat );
}

template <typename _BrightnessType = uint8_t>
class Colorizer;

template <int Hueing> class HueingColorizer;


}
}

inline dStorm::Pixel operator*( const dStorm::viewer::ColourSchemes::RGBWeight& r, uint8_t brightness );
inline dStorm::Pixel operator*( uint8_t brightness, const dStorm::viewer::ColourSchemes::RGBWeight& r );


#endif
