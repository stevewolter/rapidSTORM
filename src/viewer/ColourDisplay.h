#ifndef DSTORM_TRANSMISSIONS_COLOURDISPLAY_H
#define DSTORM_TRANSMISSIONS_COLOURDISPLAY_H

#include "Colorizer_decl.h"

#include <dStorm/outputs/BinnedLocalizations.h>
#include "Config.h"
#include <limits>
#include <dStorm/Pixel.h>
#include <dStorm/helpers/DisplayDataSource.h>

namespace dStorm {
namespace viewer {

template <typename _BrightnessType>
class Colorizer 
    : public outputs::DummyBinningListener 
{
  public:
    typedef _BrightnessType BrightnessType;

  private:
    bool invert;
  protected:
    Pixel inv( Pixel p ) const {
        if ( invert )
            return p.invert();
        else
            return p;
    }
  public:
    static const int BrightnessDepth 
        = (1U << (sizeof(BrightnessType) * 8));
    static const int KeyCount = 1;

    Colorizer( const Config& config ) 
        : invert( config.invert() ) {}

    Pixel getPixel( int x, int y, 
                    BrightnessType brightness ) const;
    /** Get the brightness for a pixel in the key. */
    Pixel getKeyPixel( BrightnessType brightness ) const;
    Pixel get_background() const { return inv(Pixel(0)); }

    dStorm::Display::KeyDeclaration create_key_declaration( int index ) const
        { throw std::logic_error("Request to declare unknown key"); }
    void create_full_key( dStorm::Display::Change::Keys::value_type& into, int index ) const
        { throw std::logic_error("Request to write unknown key"); }
};

}
}

#endif
