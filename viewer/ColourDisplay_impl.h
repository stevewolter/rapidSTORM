#ifndef DSTORM_TRANSMISSIONS_COLOURDISPLAY_IMPL_H
#define DSTORM_TRANSMISSIONS_COLOURDISPLAY_IMPL_H

#include "ColourDisplay.h"
#include <CImg.h>
#include <Eigen/Core>

namespace dStorm {
namespace viewer {

template <> 
class HueingColorizer<ColourSchemes::BlackWhite>
: public Colorizer<unsigned char>
{
  public:
    typedef Colorizer<unsigned char>::BrightnessType BrightnessType;
    HueingColorizer(const Viewer::Config& c)
        : Colorizer<unsigned char>(c) {} 
    inline Pixel getPixel( int, int, BrightnessType br ) 
            { return inv( Pixel(br) ); }
    inline Pixel getKeyPixel( BrightnessType br ) 
        { return getPixel(0, 0, br ); }
};

template <> 
class HueingColorizer<ColourSchemes::BlackRedYellowWhite>
: public Colorizer<unsigned short>
{
  public:
    typedef unsigned short BrightnessType;
    static const int BrightnessDepth = 0x300;

    HueingColorizer(const Viewer::Config& c)
        : Colorizer<unsigned short>(c) {} 
    Pixel getPixel( int, int, BrightnessType br ) {
        unsigned char part = br & 0xFF;
        return inv(
            ( br < 0x100 ) ? Pixel( part, 0, 0 ) :
            ( br < 0x200 ) ? Pixel( 0xFF, part, 0 ) :
                             Pixel( 0xFF, 0xFF, part ) );
    }
    inline Pixel getKeyPixel( BrightnessType br ) 
        { return getPixel(0, 0, br ); }
};

template <> 
class HueingColorizer<ColourSchemes::FixedHue> 
: public Colorizer<unsigned char>
{
    float rgb_weights[3];

  public:
    typedef Colorizer<unsigned char>::BrightnessType BrightnessType;
    HueingColorizer(const Viewer::Config& config)
    : Colorizer<unsigned char>(config) {
        ColourSchemes::rgb_weights_from_hue_saturation
            ( config.hue(), config.saturation(),
              rgb_weights, 1 );
    }
    Pixel getPixel( int, int, BrightnessType val ) 
    {
        return inv( Pixel(
            rgb_weights[0] * val,
            rgb_weights[1] * val,
            rgb_weights[2] * val ) );
    }
    inline Pixel getKeyPixel( BrightnessType br ) 
        { return getPixel(0, 0, br ); }
};

template <int Hueing> 
class HueingColorizer : public Colorizer<unsigned char> { 
    typedef Eigen::Matrix<float,2,1,Eigen::DontAlign> ColourVector;
  public:
    typedef Colorizer<unsigned char>::BrightnessType BrightnessType;

  private:
    float minV, maxV, scaleV;

    /** Offset for tone calculations. */
    ColourVector base_tone,
    /** Tone for currently processed points in radial Hue/Sat space */
                 current_tone;
    /** Tone for currently processed points in cartesian Hue/Sat space. */
    ColourVector tone_point;

    cimg_library::CImg<ColourVector> colours;
    cimg_library::CImg<float> rgb_weights;

    void set_tone( float hue, float saturation ) {
        current_tone = base_tone + ColourVector( hue, saturation );
        tone_point.x() = cosf( 2 * M_PI * current_tone[0] ) 
                            * current_tone[1];
        tone_point.y() = sinf( 2 * M_PI * current_tone[0] ) 
                            * current_tone[1];
    }

    void merge_tone( int x, int y, 
                     float old_data_weight, float new_data_weight )
    {
        assert( int(colours.width) > x && int(colours.height) > y );
        ColourVector hs;
        if ( old_data_weight < 1E-3 ) {
            colours(x,y) = tone_point;
            hs = current_tone;
        } else {
            colours(x,y) = 
                ( colours(x,y) * old_data_weight + 
                  tone_point * new_data_weight ) /
                  ( old_data_weight + new_data_weight );
            ColourSchemes::convert_xy_tone_to_hue_sat
                ( colours(x,y).x(), colours(x,y).y(), hs[0], hs[1] );
        }
        int pixels_per_colour = rgb_weights.width * rgb_weights.height;
        ColourSchemes::rgb_weights_from_hue_saturation
            ( hs[0], hs[1], &rgb_weights(x,y), pixels_per_colour );
                
    }

  public:
    HueingColorizer(const Viewer::Config& config)
        : Colorizer<unsigned char>(config)
        { base_tone[0] = config.hue(); 
          base_tone[1] = config.saturation(); } 

    void setSize( const input::Traits<outputs::BinnedImage>& traits ) {
        int px_w = traits.size.x() / cs_units::camera::pixel,
            px_h = traits.size.y() / cs_units::camera::pixel;
        colours = cimg_library::CImg<ColourVector> (px_w, px_h);
        rgb_weights.resize( px_w, px_h, 1, 3, -1 );
    }

    inline Pixel getPixel( int x, int y, 
                           BrightnessType val ) 
    {
        return inv( Pixel(
            round(val * rgb_weights(x,y,0,0)),
            round(val * rgb_weights(x,y,0,1)),
            round(val * rgb_weights(x,y,0,2))
        ) );
    }
    Pixel getKeyPixel( BrightnessType val ) { return inv( Pixel( val ) ); }

    inline void updatePixel(int x, int y, 
                            float oldVal, float newVal) 
        { merge_tone(x, y, oldVal, newVal - oldVal); }
    inline void announce(const output::Output::Announcement& a) {
        minV = 0;
        if ( a.traits.total_frame_count.is_set() )
            maxV = a.traits.total_frame_count->value();
        else
            throw std::runtime_error("Total length of acquisition must be "
                                     "known for colour coding by time.");
        scaleV = 2.0 / (3 * ( maxV - minV ));
    }
    inline void announce(const output::Output::EngineResult& er) {
        float hue = (er.forImage.value() - minV) * scaleV,
              saturation = 0;
        set_tone( hue, saturation );
    }
    inline void announce(const Localization&) {}
};

}
}

#endif
