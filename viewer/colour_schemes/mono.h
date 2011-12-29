#ifndef DSTORM_VIEWER_COLOUR_SCHEMES_MONO_H
#define DSTORM_VIEWER_COLOUR_SCHEMES_MONO_H

#include "base.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

class Mono
: public Base<unsigned char>
{
  public:
    typedef Base<unsigned char>::BrightnessType BrightnessType;
    Mono(bool invert)
        : Base<unsigned char>(invert) {} 
    inline Pixel getPixel( Im::Position, BrightnessType br ) const
            { return inv( Pixel(br) ); }
    inline Pixel getKeyPixel( BrightnessType br ) const
        { return getPixel(Im::Position::Zero(), br ); }
};

}
}
}

#endif
