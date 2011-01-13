#ifndef DSTORM_VIEWER_COLOUR_SCHEMES_COLORED_H
#define DSTORM_VIEWER_COLOUR_SCHEMES_COLORED_H

#include "base.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

class Colored
: public Base<unsigned char>
{
    RGBWeight weights;

  public:
    typedef Base<unsigned char>::BrightnessType BrightnessType;
    Colored(bool invert, double hue, double saturation)
    : Base<unsigned char>(invert) {
        rgb_weights_from_hue_saturation
            ( hue, saturation, weights );
    }
    Pixel getPixel( int, int, BrightnessType val )  const
    {
        return inv( weights * val );
    }
    inline Pixel getKeyPixel( BrightnessType br )  const
        { return getPixel(0, 0, br ); }
};

}
}
}

#endif
