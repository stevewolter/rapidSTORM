#ifndef DSTORM_VIEWER_COLOUR_SCHEMES_MONO_H
#define DSTORM_VIEWER_COLOUR_SCHEMES_MONO_H

#include "base.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

class Mono
: public Base
{
    virtual Mono* clone_() const { return new Mono(*this); }
  public:
    Mono(bool invert)
        : Base(invert) {} 
    Pixel getPixel( Im::Position, BrightnessType br ) const
            { return inv( Pixel(br) ); }
    Pixel getKeyPixel( BrightnessType br ) const
        { return getPixel(Im::Position::Zero(), br ); }
};

}
}
}

#endif
