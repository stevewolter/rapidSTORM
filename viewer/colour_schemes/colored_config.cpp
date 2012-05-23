#include "colored.h"
#include "../Config.h"
#include "base_impl.h"
#include <simparm/Object.hh>
#include <simparm/Entry.hh>
#include <dStorm/helpers/default_on_copy.h>

namespace dStorm {
namespace viewer {
namespace colour_schemes {

struct ColoredConfig : public ColourScheme
{
    simparm::Entry<double> hue, saturation;
    simparm::BaseAttribute::ConnectionStore listening[2];
    default_on_copy< boost::signals2::signal<void()> > change;

    ColoredConfig();
    ColoredConfig(const ColoredConfig&);
    ColoredConfig* clone() const { return new ColoredConfig(*this); }
    std::auto_ptr<Backend> make_backend( Config&, Status& ) const;
    void add_listener( simparm::BaseAttribute::Listener );
    void attach_ui( simparm::NodeHandle );
};

ColoredConfig::ColoredConfig() 
: ColourScheme("FixedHue", "Constant colour"),
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
    hue.helpID = "#Viewer_Hue";
    saturation.helpID = "#Viewer_Saturation";

    saturation.min = (0);
    saturation.max = (1);
    saturation.setHelp("Select a saturation between 0 and 1 for the color "
                       "in the display. Saturation 0 means no color (pure "
                       "black to pure white) and 1 means fully saturated "
                       "color.");
}

ColoredConfig::ColoredConfig(const ColoredConfig& o)
: ColourScheme(o),
  hue(o.hue), saturation(o.saturation)
{
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

std::auto_ptr<Backend> ColoredConfig::make_backend( Config& config, Status& status ) const
{
    return Backend::create< Colored >(Colored(config.invert(), hue(), saturation()), status);
}

}

template <>
std::auto_ptr<ColourScheme> ColourScheme::config_for<colour_schemes::Colored>()
{
    return std::auto_ptr<ColourScheme>(new colour_schemes::ColoredConfig());
}

}
}
