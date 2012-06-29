#include "base.h"
#include "viewer/ColourSchemeFactory.h"
#include <simparm/Object.h>

namespace dStorm {
namespace viewer {
namespace colour_schemes {

class Greyscale
: public ColourScheme
{
    virtual Greyscale* clone_() const { return new Greyscale(*this); }
  public:
    Greyscale(bool invert)
        : ColourScheme(invert) {} 
    Pixel getPixel( Im::Position, BrightnessType br ) const
            { return inv( Pixel(br) ); }
    Pixel getKeyPixel( BrightnessType br ) const
        { return getPixel(Im::Position::Zero(), br ); }
};

struct GreyscaleConfig : public ColourSchemeFactory
{
    GreyscaleConfig()
        : ColourSchemeFactory("BlackWhite", "Greyscale") {}
    GreyscaleConfig* clone() const { return new GreyscaleConfig(*this); }
    std::auto_ptr<ColourScheme> make_backend( bool invert ) const
        { return std::auto_ptr<ColourScheme>(new Greyscale(invert)); }
    void attach_ui( simparm::NodeHandle at ) { attach_parent(at); }
    void add_listener( simparm::BaseAttribute::Listener ) {}
};

std::auto_ptr<ColourSchemeFactory> make_greyscale_factory()
{
    return std::auto_ptr<ColourSchemeFactory>(new colour_schemes::GreyscaleConfig());
}

}
}
}
