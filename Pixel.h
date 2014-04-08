#ifndef DSTORM_PIXEL_H
#define DSTORM_PIXEL_H

#include <stdint.h>
#include <cmath>
#include <limits>

namespace dStorm {

class Pixel {
    union Contents {
        uint8_t colors[4];
        uint32_t _value;
        Contents() : _value(0) {}
        Contents( uint32_t v ) : _value(v) {}
        Contents( const Contents& o ) : _value(o._value) {}
    };
    Contents c;
    explicit Pixel( uint32_t _value, bool ) : c(_value) {}
    static const int maxVal = 255;
  public:
    Pixel() { c._value = 0; alpha() = maxVal; }
    Pixel( uint8_t grey_level ) {
        red() = green() = blue() = grey_level;
        alpha() = maxVal;
    }
    Pixel( uint8_t red, uint8_t green, uint8_t blue, 
           uint8_t alpha = maxVal ) 
    {
        this->red() = red;
        this->green() = green;
        this->blue() = blue;
        this->alpha() = alpha;
    }

    uint8_t red() const { return c.colors[0]; }
    uint8_t& red() { return c.colors[0]; }
    uint8_t green() const { return c.colors[1]; }
    uint8_t& green() { return c.colors[1]; }
    uint8_t blue() const { return c.colors[2]; }
    uint8_t& blue() { return c.colors[2]; }
    uint8_t alpha() const { return c.colors[3]; }
    uint8_t& alpha() { return c.colors[3]; }
    uint8_t sum() const { return red()+green()+blue()+alpha(); }
    operator uint8_t () const 
        { return (int(red())+int(green())+int(blue())) / 3; }

    Pixel invert() const {return Pixel( ~c._value, true); }

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

    int distance(const Pixel& other) const {
        int d = 0;
        for (int i = 0; i < 4; ++i) 
            if ( c.colors[i] > other.c.colors[i] )
                d += c.colors[i] - other.c.colors[i];
            else
                d += other.c.colors[i] - c.colors[i];
        return d;
    }
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
