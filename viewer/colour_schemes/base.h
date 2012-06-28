#ifndef DSTORM_TRANSMISSIONS_COLOURDISPLAY_BASE_H
#define DSTORM_TRANSMISSIONS_COLOURDISPLAY_BASE_H

#include <dStorm/outputs/BinnedLocalizations.h>
#include <limits>
#include <dStorm/Pixel.h>
#include <dStorm/display/DataSource.h>
#include <boost/array.hpp>
#include <stdint.h>
#include "../Image.h"
#include "density_map/DummyListener.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

typedef boost::array<float,3> RGBWeight;

inline dStorm::Pixel operator*( const RGBWeight& r, uint8_t brightness );
inline dStorm::Pixel operator*( uint8_t brightness, const RGBWeight& r );

void rgb_weights_from_hue_saturation
    ( float hue, float saturation, RGBWeight& weight );
void convert_xy_tone_to_hue_sat( 
    float x, float y, float& hue, float& sat );

class Base 
    : public density_map::DummyListener<Im::Dim>
{
  public:
    typedef uint_fast16_t BrightnessType;

  private:
    bool invert;
    virtual Base* clone_() const = 0;
  protected:
    Pixel inv( Pixel p ) const {
        if ( invert )
            return p.invert();
        else
            return p;
    }
  public:
    static const int KeyCount = 1;

    Base( bool invert ) 
        : invert( invert ) {}
    std::auto_ptr<Base> clone() const { return std::auto_ptr<Base>( clone_() ); }
    virtual ~Base() {}

    virtual Pixel getPixel( Im::Position, BrightnessType brightness ) const = 0;
    /** Get the brightness for a pixel in the key. */
    virtual Pixel getKeyPixel( BrightnessType brightness ) const = 0;
    Pixel get_background() const { return inv(Pixel(0)); }

    virtual dStorm::display::KeyDeclaration create_key_declaration( int index ) const
        { throw std::logic_error("Request to declare unknown key"); }
    virtual void create_full_key( dStorm::display::Change::Keys::value_type& into, int index ) const
        { throw std::logic_error("Request to write unknown key"); }
    virtual void notice_user_key_limits(int, bool, std::string)
        { throw std::logic_error("Request to set limits for unknown key"); }
    virtual const int brightness_depth() const { return 0x100; }
};

}
}
}

#endif
