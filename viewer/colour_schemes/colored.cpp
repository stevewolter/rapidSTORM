#include "viewer/ColourScheme.h"
#include "viewer/ColourSchemeFactory.h"
#include "viewer/colour_schemes/base.h"
#include <simparm/Object.h>
#include <simparm/Entry.h>
#include <dStorm/helpers/default_on_copy.h>

namespace dStorm {
namespace viewer {
namespace colour_schemes {

class Colored
: public ColourScheme
{
    RGBWeight weights;
    virtual Colored* clone_() const { return new Colored(*this); }

  public:
    Colored(bool invert, double hue, double saturation)
    : ColourScheme(invert) {
        rgb_weights_from_hue_saturation
            ( hue, saturation, weights );
    }
    Pixel getPixel( Im::Position, BrightnessType val )  const
    {
        return inv( weights * val );
    }
    inline Pixel getKeyPixel( BrightnessType br )  const
        { return getPixel(Im::Position::Zero(), br ); }
};

struct ColoredConfig : public ColourSchemeFactory
{
    simparm::Entry<double> hue, saturation;
    simparm::BaseAttribute::ConnectionStore listening[2];
    default_on_copy< boost::signals2::signal<void()> > change;

    ColoredConfig();
    ColoredConfig* clone() const { return new ColoredConfig(*this); }
    std::auto_ptr<ColourScheme> make_backend( bool invert ) const;
    void add_listener( simparm::BaseAttribute::Listener );
    void attach_ui( simparm::NodeHandle );
};

ColoredConfig::ColoredConfig() 
: ColourSchemeFactory("FixedHue", "Constant colour"),
  hue("Hue", "Select color hue", 0),
  saturation("Saturation", "Select saturation", 1)
{
    hue.min = (0);
    hue.max = (1);
    hue.setHelp("Select a hue between 0 and 1 to display localizations in."
                " The hue is selected along the HSV color axis, following "
                "the natural spectrum from 0 (red) over 1/6 (yellow), "
                "1/3 (green), 1/2 (cyan), 2/3 (blue) to 5/6 (violet) and "
                "1 (red again)");
    hue.setHelpID( "#Viewer_Hue" );
    saturation.setHelpID( "#Viewer_Saturation" );

    saturation.min = (0);
    saturation.max = (1);
    saturation.setHelp("Select a saturation between 0 and 1 for the color "
                       "in the display. Saturation 0 means no color (pure "
                       "black to pure white) and 1 means fully saturated "
                       "color.");
}

void ColoredConfig::attach_ui( simparm::NodeHandle at ) {
    listening[0] = hue.value.notify_on_value_change( change );
    listening[1] = saturation.value.notify_on_value_change( change );

    simparm::NodeHandle r = attach_parent(at);
    hue.attach_ui(r);
    saturation.attach_ui(r);
}

void ColoredConfig::add_listener( simparm::BaseAttribute::Listener l ) {
    change.connect(l);
}

std::auto_ptr<ColourScheme> ColoredConfig::make_backend( bool invert ) const
{
    return std::auto_ptr<ColourScheme>(new Colored(invert, hue(), saturation()));
}

std::auto_ptr<ColourSchemeFactory> make_colored_factory()
{
    return std::auto_ptr<ColourSchemeFactory>(new colour_schemes::ColoredConfig());
}

}
}
}
