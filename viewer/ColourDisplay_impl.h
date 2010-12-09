#ifndef DSTORM_TRANSMISSIONS_COLOURDISPLAY_IMPL_H
#define DSTORM_TRANSMISSIONS_COLOURDISPLAY_IMPL_H

#include "ColourDisplay.h"
#include <Eigen/Core>
#include <boost/units/io.hpp>
#include <dStorm/output/ResultRepeater.h>
#include "HueSaturationMixer.h"

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
class HueingVariable;

template <int Hueing> 
class HueingColorizer : public Colorizer<unsigned char>, public HueSaturationMixer { 
  public:
    typedef Colorizer<unsigned char> BaseType;
    typedef BaseType::BrightnessType BrightnessType;

    static const int KeyCount = 2;

  private:
    typedef typename HueingVariable<Hueing>::Unit Unit;
    typedef quantity<Unit,float> Qty;

    HueingVariable<Hueing> variable;

    Qty origrange[2], range[2];
    simparm::optional<Qty> userrange[2];
    typedef typename boost::units::power_typeof_helper<Unit,static_rational<-1> >::type per_frame;
    quantity<per_frame,float> scaleV;
    static const int key_resolution = 100;

    output::ResultRepeater *repeater;

    void set_user_limit(int lower, simparm::optional<Qty> value);
    void set_tone( quantity<Unit,float> value ) {
        Qty clipped = std::max(range[0], std::min(value, range[1]));
        float scaledV = ( clipped - range[0] ) * scaleV;
        HueSaturationMixer::set_tone( scaledV );
    }
    void set_range()
    {
        for (int i = 0; i < 2; ++i )
            range[i] = ( userrange[i].is_set() ) ? *userrange[i] : origrange[i]; 
        scaleV = 0.666f / (range[1] - range[0]);
    }

    inline float
    key_value( quantity<Unit,float> ) const;

  public:
    HueingColorizer(const Config& config) 
        : BaseType(config), HueSaturationMixer(config), variable(config),
          repeater(NULL) {}

    void setSize( const input::Traits<outputs::BinnedImage>& traits ) {
        BaseType::setSize(traits);
        HueSaturationMixer::setSize(traits.size);
    }
    Pixel getPixel(int x, int y, BrightnessType val)
        { return inv( HueSaturationMixer::getPixel(x,y,val) ); }
    Pixel getKeyPixel( unsigned char val ) const 
        { return inv( HueSaturationMixer::getKeyPixel(val) ); }
    void updatePixel(int x, int y, float oldVal, float newVal) 
        { HueSaturationMixer::updatePixel(x,y, oldVal, newVal); }

    void announce(const output::Output::Announcement& a); 
    inline void announce(const output::Output::EngineResult& er);
    inline void announce(const Localization&);

    template <typename Unit>
    dStorm::Display::KeyDeclaration key_declaration() const {
        dStorm::Display::KeyDeclaration rv(
            boost::units::symbol_string( Unit() ),
            boost::units::name_string( Unit() ),
            key_resolution);
        if ( repeater ) {
            rv.can_set_lower_limit = rv.can_set_upper_limit = true;
            std::stringstream s[2];
            for (int i = 0; i < 2; ++i )
                s[i] << key_value( range[i] );
            rv.lower_limit = s[0].str(); rv.upper_limit = s[1].str();
        }
        return rv;
    }
    inline dStorm::Display::KeyDeclaration create_key_declaration( int index ) const;

    void create_full_key( dStorm::Display::Change::Keys::value_type& into, int index ) const;

    void notice_user_key_limits(int, bool, std::string);
};

template <>
struct HueingVariable<ColourSchemes::TimeHue>
{
    typedef cs_units::camera::time Unit;
    typedef boost::units::si::time AlternateUnit;

    simparm::optional<frame_rate> speed;

    HueingVariable( const Config& ) {}
};

template <>
struct HueingVariable<ColourSchemes::ZHue>
{
    typedef boost::units::si::nanolength Unit;

    HueingVariable( const Config& ) {}
};

template <>
inline float
HueingColorizer<ColourSchemes::TimeHue>
::key_value( quantity<Unit,float> frame ) const {
    if ( variable.speed.is_set() ) {
        quantity<si::time, double> time = frame / *variable.speed;
        return time.value();
    } else {
        return frame.value();
    }
}

template <int Hueing>
inline float
HueingColorizer<Hueing>
::key_value( quantity<Unit,float> frame ) const {
    return frame.value();
}

template <>
inline dStorm::Display::KeyDeclaration 
HueingColorizer<ColourSchemes::TimeHue>
::create_key_declaration( int index ) const
{
    if ( index == 1 ) {
        if ( variable.speed.is_set() )
            return key_declaration<HueingVariable<ColourSchemes::TimeHue>::AlternateUnit>();
        else
            return key_declaration<Unit>();
    } else {
        return BaseType::create_key_declaration( index );
    }
}

template <int Hueing>
inline dStorm::Display::KeyDeclaration 
HueingColorizer<Hueing>
::create_key_declaration( int index ) const
{
    if ( index == 1 ) {
        return key_declaration<Unit>();
    } else {
        return BaseType::create_key_declaration( index );
    }
}

template <>
inline void HueingColorizer<ColourSchemes::TimeHue>
::announce(const output::Output::EngineResult& er)
{
    set_tone( er.forImage );
}
template <>
inline void HueingColorizer<ColourSchemes::TimeHue>
::announce(const Localization& l)
{
}

template <>
inline void HueingColorizer<ColourSchemes::ZHue>
::announce(const output::Output::EngineResult& er)
{
}

template <>
inline void HueingColorizer<ColourSchemes::ZHue>
::announce(const Localization& l)
{
    set_tone( l.zposition() );
}

}
}

#endif
