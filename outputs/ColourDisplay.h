#ifndef DSTORM_TRANSMISSIONS_COLOURDISPLAY_H
#define DSTORM_TRANSMISSIONS_COLOURDISPLAY_H

#include <outputs/BinnedLocalizations.h>
#include <outputs/ViewerConfig.h>
#include <limits>

namespace dStorm {

namespace ColourSchemes {
    enum Scheme { BlackWhite, BlackRedYellowWhite,
                  FixedHue, TimeHue, ExtraHue, ExtraSaturation,
                  FirstColourModel = BlackWhite,
                  LastColourModel = ExtraSaturation};

    void rgb_weights_from_hue_saturation
        ( float hue, float saturation, float *weightv, int step ) 
;
    void convert_xy_tone_to_hue_sat( 
        float x, float y, float& hue, float& sat );
};

template <typename _BrightnessType = unsigned char>
class Colorizer 
    : public DummyBinningListener 
{
  public:
    typedef _BrightnessType BrightnessType;
    struct Pixel { 
        uint8_t r, g, b;
        Pixel(uint8_t grey_value) 
            : r(grey_value), g(grey_value), b(grey_value){}
        Pixel(uint8_t r, uint8_t g, uint8_t b) 
            : r(r), g(g), b(b) {}
    };

  private:
    bool invert;
  protected:
    Pixel inv( Pixel p ) {
        if ( invert )
            return Pixel( ~p.r, ~p.g, ~p.b );
        else
            return p;
    }
  public:
    static const int BrightnessDepth 
        = (1U << (sizeof(BrightnessType) * 8));

    Colorizer( const Viewer::Config& config ) 
        : invert( config.invert() ) {}

    Pixel getPixel( int x, int y, 
                    BrightnessType brightness );
    /** Get the brightness for a pixel in the key. */
    Pixel getKeyPixel( BrightnessType brightness );
    Pixel get_background() { return inv(Pixel(0)); }
};

template <int Hueing> class HueingColorizer;

}

#endif
