#ifndef DSTORM_PIXEL_H
#define DSTORM_PIXEL_H

#include <stdint.h>

namespace dStorm {

class Pixel {
    uint8_t _red, _green, _blue, _alpha;
    static const int maxVal = 255;
  public:
    Pixel() : _red(0), _green(0), _blue(0), _alpha(maxVal) {}
    Pixel( uint8_t grey_level ) 
        : _red(grey_level), _green(grey_level), _blue(grey_level),
          _alpha(maxVal) {}
    Pixel( uint8_t red, uint8_t green, uint8_t blue, 
           uint8_t alpha = maxVal ) 
        : _red(red), _green(green), _blue(blue), _alpha(alpha) {}

    uint8_t red() const { return _red; }
    uint8_t& red() { return _red; }
    uint8_t green() const { return _green; }
    uint8_t& green() { return _green; }
    uint8_t blue() const { return _blue; }
    uint8_t& blue() { return _blue; }
    uint8_t alpha() const { return _alpha; }
    uint8_t& alpha() { return _alpha; }

    Pixel invert() const {return Pixel( ~_red, ~_green, ~_blue, _alpha );}

    static Pixel Red() { return Pixel( maxVal, 0, 0 ); }
    static Pixel Green() { return Pixel( 0, maxVal, 0 ); }
    static Pixel Blue() { return Pixel( 0, 0, maxVal ); }
    static Pixel Black() { return Pixel( 0 ); }
    static Pixel White() { return Pixel( maxVal ); }
};

}

#endif
