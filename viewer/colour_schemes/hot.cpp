#include "base.h"
#include <viewer/ColourScheme.h>
#include <viewer/ColourSchemeFactory.h>
#include <simparm/Object.h>

namespace dStorm {
namespace viewer {
namespace colour_schemes {

class Hot
: public ColourScheme
{
    virtual Hot* clone_() const { return new Hot(*this); }
  public:
    Hot(bool invert)
        : ColourScheme(invert) {} 
    Pixel getPixel( Im::Position, BrightnessType br ) const {
        unsigned char part = br & 0xFF;
        return inv(
            ( br < 0x100 ) ? Pixel( part, 0, 0 ) :
            ( br < 0x200 ) ? Pixel( 0xFF, part, 0 ) :
                             Pixel( 0xFF, 0xFF, part ) );
    }
    inline Pixel getKeyPixel( BrightnessType br )  const
        { return getPixel(Im::Position::Zero(), br ); }
    const int brightness_depth() const { return 0x300; }
};

struct HotConfig : public ColourSchemeFactory
{
    HotConfig()
        : ColourSchemeFactory("BlackRedYellowWhite", "Colour code ranging from red over yellow to white") {}
    HotConfig* clone() const { return new HotConfig(*this); }
    std::auto_ptr<ColourScheme> make_backend( bool invert ) const
        { return std::auto_ptr<ColourScheme>(new Hot(invert)); }
    void attach_ui( simparm::NodeHandle at ) { attach_parent(at); }
    void add_listener( simparm::BaseAttribute::Listener ) {}
};

std::auto_ptr<ColourSchemeFactory> make_hot_factory()
{
    return std::auto_ptr<ColourSchemeFactory>(new colour_schemes::HotConfig());
}

}
}
}
