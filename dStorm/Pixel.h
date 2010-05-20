#ifndef DSTORM_PIXEL_H
#define DSTORM_PIXEL_H

#include <stdint.h>
#include <limits>

namespace dStorm {

class Pixel {
    union Contents {
        uint8_t _red, _green, _blue, _alpha;
        uint32_t _value;
    };
    Contents c;
    explicit Pixel( uint32_t _value, bool )
        { c._value = _value; }
    static const int maxVal = 255;
  public:
    Pixel() { c._value = 0; c._alpha = maxVal; }
    Pixel( uint8_t grey_level ) {
        c._red = c._green = c._blue = grey_level;
        c._alpha = maxVal;
    }
    Pixel( uint8_t red, uint8_t green, uint8_t blue, 
           uint8_t alpha = maxVal ) 
    {
        c._red = red;
        c._green = green;
        c._blue = blue;
        c._alpha = (alpha);
    }

    uint8_t red() const { return c._red; }
    uint8_t& red() { return c._red; }
    uint8_t green() const { return c._green; }
    uint8_t& green() { return c._green; }
    uint8_t blue() const { return c._blue; }
    uint8_t& blue() { return c._blue; }
    uint8_t alpha() const { return c._alpha; }
    uint8_t& alpha() { return c._alpha; }
    uint8_t sum() const { return c._red+c._green+c._blue+c._alpha; }
    operator uint8_t () const 
        { return (c._red+c._green+c._blue) / 3; }

    Pixel invert() const {return Pixel( ~c._red, ~c._green, ~c._blue, c._alpha );}

    static Pixel Red() { return Pixel( maxVal, 0, 0 ); }
    static Pixel Green() { return Pixel( 0, maxVal, 0 ); }
    static Pixel Blue() { return Pixel( 0, 0, maxVal ); }
    static Pixel Black() { return Pixel( 0 ); }
    static Pixel White() { return Pixel( maxVal ); }

    Pixel operator+(const Pixel& p) const 
        { return Pixel(c._value + p.c._value, true); }
    Pixel operator-(const Pixel& p) const 
        { return Pixel(c._value - p.c._value, true); }
    Pixel operator*(float f) const 
        { return Pixel(c._value * f, true); }
    Pixel operator*(const Pixel& p) const 
        { return Pixel(c._value * p.c._value, true); }
    Pixel operator/(float f) const 
        { return Pixel(c._value / f, true); }
    Pixel operator/(const Pixel& p) const 
        { return Pixel(c._value / p.c._value, true); }
};

inline bool operator<(const Pixel& a, const Pixel&b) {
    return a.sum() < b.sum();
}
inline bool operator>(const Pixel& a, const Pixel&b) {
    return a.sum() > b.sum();
}
inline bool operator>=(const Pixel& a, const Pixel&b) {
    return a.sum() >= b.sum();
}
inline bool operator<=(const Pixel& a, const Pixel&b) {
    return a.sum() <= b.sum();
}

}

namespace std {
template <>
struct numeric_limits<dStorm::Pixel> 
: public numeric_limits<uint8_t> {
    static dStorm::Pixel max() { return dStorm::Pixel(255); }
    static dStorm::Pixel min() { return dStorm::Pixel(0); }
};
}

#endif
