#ifndef DSTORM_VIEWER_COLOUR_SCHEMES_COLORED_H
#define DSTORM_VIEWER_COLOUR_SCHEMES_COLORED_H

#include "base.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

class Colored
: public Base
{
    RGBWeight weights;
    virtual Colored* clone_() const { return new Colored(*this); }

  public:
    Colored(bool invert, double hue, double saturation)
    : Base(invert) {
        rgb_weights_from_hue_saturation
            ( hue, saturation, weights );
    }
    Pixel getPixel( Im::Position, BrightnessType val )  const
    {
        return inv( weights * val );
    }
    inline Pixel getKeyPixel( BrightnessType br )  const
        { return getPixel(Im::Position::Zero(), br ); }
};

}
}
}

#endif
