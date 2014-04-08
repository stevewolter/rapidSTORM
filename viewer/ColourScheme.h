#ifndef DSTORM_VIEWER_COLOURSCHEME_H
#define DSTORM_VIEWER_COLOURSCHEME_H

#include "density_map/VirtualListener.h"
#include <memory>
#include "Pixel.h"
#include "display/DataSource.h"
#include "viewer/Image.h"

namespace dStorm {
namespace viewer {

class ColourScheme
    : public density_map::VirtualListener<Im::Dim>
{
  public:
    typedef uint_fast16_t BrightnessType;

  private:
    bool invert;
    virtual ColourScheme* clone_() const = 0;
  protected:
    Pixel inv( Pixel p ) const {
        if ( invert )
            return p.invert();
        else
            return p;
    }
  public:
    ColourScheme( bool invert ) 
        : invert( invert ) {}
    std::auto_ptr<ColourScheme> clone() const { return std::auto_ptr<ColourScheme>( clone_() ); }
    virtual ~ColourScheme() {}

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
    virtual int key_count() const { return 1; }

    void setSize( const MetaInfo& ) {}
    void announce(const output::Output::Announcement& a) {}
    void announce(const output::Output::EngineResult& er) {}
    void announce(const Localization&) {}
    void clean( bool ) {}
    void clear() {}
    void updatePixel(const Im::Position&, float, float) {}
};


}
}

#endif
