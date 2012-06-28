#ifndef DSTORM_VIEWER_COLOUR_SCHEMES_HOT_H
#define DSTORM_VIEWER_COLOUR_SCHEMES_HOT_H

#include "base.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

class Hot
: public Base
{
    virtual Hot* clone_() const { return new Hot(*this); }
  public:
    Hot(bool invert)
        : Base(invert) {} 
    Pixel getPixel( Im::Position, BrightnessType br ) const {
        unsigned char part = br & 0xFF;
        return inv(
            ( br < 0x100 ) ? Pixel( part, 0, 0 ) :
            ( br < 0x200 ) ? Pixel( 0xFF, part, 0 ) :
                             Pixel( 0xFF, 0xFF, part ) );
    }
    inline Pixel getKeyPixel( BrightnessType br )  const
        { return getPixel(Im::Position::Zero(), br ); }
    const int brightness_depth() const { return 0x300; }
};

}
}
}

#endif
