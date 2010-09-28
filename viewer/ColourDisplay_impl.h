#ifndef DSTORM_TRANSMISSIONS_COLOURDISPLAY_IMPL_H
#define DSTORM_TRANSMISSIONS_COLOURDISPLAY_IMPL_H

#include "ColourDisplay.h"
#include <Eigen/Core>
#include <dStorm/Image.h>
#include <dStorm/image/constructors.h>

inline dStorm::Pixel operator*( const dStorm::viewer::ColourSchemes::RGBWeight& r, uint8_t b ) {
    return dStorm::Pixel( round(r[0] * b), round(r[1] * b), round(r[2] * b) );
}
inline dStorm::Pixel operator*( uint8_t b, const dStorm::viewer::ColourSchemes::RGBWeight& r ) {
    return dStorm::Pixel( r[0] * b, r[1] * b, r[2] * b );
}

namespace dStorm {
namespace viewer {

template <> 
class HueingColorizer<ColourSchemes::BlackWhite>
: public Colorizer<unsigned char>
{
  public:
    typedef Colorizer<unsigned char>::BrightnessType BrightnessType;
    HueingColorizer(const Config& c)
        : Colorizer<unsigned char>(c) {} 
    inline Pixel getPixel( int, int, BrightnessType br ) const
            { return inv( Pixel(br) ); }
    inline Pixel getKeyPixel( BrightnessType br ) const
        { return getPixel(0, 0, br ); }
};

template <> 
class HueingColorizer<ColourSchemes::BlackRedYellowWhite>
: public Colorizer<unsigned short>
{
  public:
    typedef unsigned short BrightnessType;
    static const int BrightnessDepth = 0x300;

    HueingColorizer(const Config& c)
        : Colorizer<unsigned short>(c) {} 
    Pixel getPixel( int, int, BrightnessType br ) const {
        unsigned char part = br & 0xFF;
        return inv(
            ( br < 0x100 ) ? Pixel( part, 0, 0 ) :
            ( br < 0x200 ) ? Pixel( 0xFF, part, 0 ) :
                             Pixel( 0xFF, 0xFF, part ) );
    }
    inline Pixel getKeyPixel( BrightnessType br )  const
        { return getPixel(0, 0, br ); }
};

template <> 
class HueingColorizer<ColourSchemes::FixedHue> 
: public Colorizer<unsigned char>
{
    ColourSchemes::RGBWeight weights;

  public:
    typedef Colorizer<unsigned char>::BrightnessType BrightnessType;
    HueingColorizer(const Config& config)
    : Colorizer<unsigned char>(config) {
        ColourSchemes::rgb_weights_from_hue_saturation
            ( config.hue(), config.saturation(), weights );
    }
    Pixel getPixel( int, int, BrightnessType val )  const
    {
        return inv( weights * val );
    }
    inline Pixel getKeyPixel( BrightnessType br )  const
        { return getPixel(0, 0, br ); }
};

using namespace boost::units;

template <int Hueing> 
class HueingColorizer : public Colorizer<unsigned char> { 
    typedef Eigen::Matrix<float,2,1,Eigen::DontAlign> ColourVector;
  public:
    typedef Colorizer<unsigned char> BaseType;
    typedef BaseType::BrightnessType BrightnessType;

    static const int KeyCount = 2;

  private:
    quantity<cs_units::camera::time,float> minV, maxV;
    typedef power_typeof_helper<cs_units::camera::time,static_rational<-1> >::type per_frame;
    quantity<per_frame,float> scaleV;
    static const int key_resolution = 100;
    simparm::optional<frame_rate> speed;

    /** Offset for tone calculations. */
    ColourVector base_tone,
    /** Tone for currently processed points in radial Hue/Sat space */
                 current_tone;
    /** Tone for currently processed points in cartesian Hue/Sat space. */
    ColourVector tone_point;

    typedef ColourSchemes::RGBWeight RGBWeight;

    dStorm::Image<ColourVector,2> colours;
    dStorm::Image<RGBWeight,2> rgb_weights;

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
        assert( int(colours.width_in_pixels()) > x && int(colours.height_in_pixels()) > y );
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
        ColourSchemes::rgb_weights_from_hue_saturation( hs[0], hs[1], rgb_weights(x,y) );
                
    }

  public:
    HueingColorizer(const Config& config) : BaseType(config)
        { base_tone[0] = config.hue(); 
          base_tone[1] = config.saturation(); } 

    void setSize( const input::Traits<outputs::BinnedImage>& traits ) {
        colours.invalidate();
        rgb_weights.invalidate();
        colours = dStorm::Image<ColourVector,2>(traits.size);
        rgb_weights = dStorm::Image<RGBWeight,2>(traits.size);
    }

    inline Pixel getPixel( int x, int y, 
                           BrightnessType val )  const
    {
        return inv( Pixel( val * rgb_weights(x,y) ) );
    }
    Pixel getKeyPixel( BrightnessType val ) const 
        { return inv( Pixel( val ) ); }

    inline void updatePixel(int x, int y, 
                            float oldVal, float newVal) 
        { merge_tone(x, y, oldVal, newVal - oldVal); }
    inline void announce(const output::Output::Announcement& a) {
        minV = a.traits.first_frame;
        if ( a.traits.last_frame.is_set() )
            maxV = *a.traits.last_frame;
        else
            throw std::runtime_error("Total length of acquisition must be "
                                     "known for colour coding by time.");
        quantity<cs_units::camera::time,float> length = maxV - minV;
        scaleV = 0.666f / length;
        speed = a.traits.speed;
    }
    inline void announce(const output::Output::EngineResult& er) {
        float hue = (er.forImage - minV) * scaleV,
              saturation = 0;
        set_tone( hue, saturation );
    }
    inline void announce(const Localization&) {}

    dStorm::Display::KeyDeclaration create_key_declaration( int index ) const {
        if ( index == 1 ) {
            return dStorm::Display::KeyDeclaration(
                (speed.is_set() ? "s" : "fr"), 
                (speed.is_set() ? "seconds" : "frames"), 
                key_resolution);
        } else {
            return BaseType::create_key_declaration( index );
        }
    }

    void create_full_key( dStorm::Display::Change::Keys::value_type& into, int index ) const
    {
        if ( index == 1 ) {
            const float max_saturation = 1;
            const BrightnessType max_brightness 
                = std::numeric_limits<BrightnessType>::max();
            const int key_count = key_resolution;
            into.reserve( key_count );
            for (int i = 0; i < key_count; ++i) {
                float hue = (i * 0.666f / key_count);
                RGBWeight weights;
                ColourSchemes::rgb_weights_from_hue_saturation
                    ( hue, max_saturation, weights );

                /* Key value in frames */
                quantity<cs_units::camera::time,float>
                    frame = ((maxV - minV) * ((1.0f * i) / key_count) + minV);
                float value;
                if ( speed.is_set() ) {
                    quantity<si::time, double> time = frame / *speed;
                    value = time.value();
                } else {
                    value = frame.value();
                }

                into.push_back( dStorm::Display::KeyChange(
                    /* index */ i,
                    /* color */ weights * max_brightness,
                    /* value */ value ) );
            }
        } else {
            BaseType::create_full_key( into, index );
        }
    }
};

}
}

#endif
