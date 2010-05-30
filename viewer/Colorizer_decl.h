#ifndef DSTORM_VIEWER_COLORIZER_DECL_H
#define DSTORM_VIEWER_COLORIZER_DECL_H

#include <stdint.h>

namespace dStorm {
namespace viewer {

namespace ColourSchemes {
    enum Scheme { BlackWhite, BlackRedYellowWhite,
                  FixedHue, TimeHue, ExtraHue, ExtraSaturation,
                  FirstColourModel = BlackWhite,
                  LastColourModel = ExtraSaturation};

    void rgb_weights_from_hue_saturation
        ( float hue, float saturation, float *weightv, int step );
    void convert_xy_tone_to_hue_sat( 
        float x, float y, float& hue, float& sat );
}

template <typename _BrightnessType = uint8_t>
class Colorizer;

template <int Hueing> class HueingColorizer;


}
}

#endif
