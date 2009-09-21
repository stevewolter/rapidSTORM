#ifndef DSTORM_TRANSMISSIONS_COLOURDISPLAY_IMPL_H
#define DSTORM_TRANSMISSIONS_COLOURDISPLAY_IMPL_H

#include <dStorm/transmissions/ColourDisplay.h>

namespace dStorm {

template <>
inline unsigned char ColouredImage<ColourSchemes::BlackWhite>
   ::pixelValue( int, int, int, SmoothedPixel val ) const
{
    return val;
}

template <>
inline unsigned char ColouredImage<ColourSchemes::BlackRedYellowWhite>
   ::pixelValue( int, int, int dim, SmoothedPixel val ) const
{
    if ( dim == int(val >> 8) )
        return val & 0xFF;
    else if ( dim < int(val >> 8) )
        return 0xFF;
    else
        return 0;
}

template <>
inline unsigned char ColouredImage<ColourSchemes::FixedHue>
   ::pixelValue( int, int, int dim, SmoothedPixel val ) const
{
    return data.rgb_weights[dim] * val;
}


template <>
inline void ColouredImage<ColourSchemes::TimeHue>
    ::updatePixel(int x, int y, float oldVal, float newVal)
{
    data.merge_tone(x, y, oldVal, newVal - oldVal);
}

template <>
inline unsigned char ColouredImage<ColourSchemes::TimeHue>
   ::pixelValue( int x, int y, int dim, SmoothedPixel val ) const
{
    return round(val * data.rgb_weights(x,y,0,dim));
}


template <int Hueing>
inline void ColouredImage<Hueing>
    ::updatePixel(int, int, float, float) {}
template <int Hueing>
inline unsigned char ColouredImage<Hueing>
   ::pixelValue( int, int, int, SmoothedPixel ) const 
   { return 0; }

template <int Hueing>
inline void ColouredImage<Hueing>::
    announce(const Output::Announcement&) {}

template <>
inline void ColouredImage<ColourSchemes::TimeHue>::
    announce(const Output::Announcement& a) 
{
    data.minV = 0;
    data.maxV = a.length;
    data.scaleV = 2.0 / (3 * ( data.maxV - data.minV ));
}

template <int Hueing>
inline void ColouredImage<Hueing>::
    announce(const Output::EngineResult&) {}

template <>
inline void ColouredImage<ColourSchemes::TimeHue>::
    announce(const Output::EngineResult& er) 
{
    float hue = (er.forImage - data.minV) * data.scaleV, saturation = 0;
    data.set_tone( hue, saturation );
}

}

#endif
