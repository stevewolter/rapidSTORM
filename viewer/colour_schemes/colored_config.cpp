#include "colored.h"
#include "colored_config.h"
#include "../Config.h"
#include "base_impl.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

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

    hue.attach_ui(this->node);
    saturation.attach_ui( this->node );
}

ColoredConfig::ColoredConfig(const ColoredConfig& o)
: ColourScheme(o),
  hue(o.hue), saturation(o.saturation)
{
    hue.attach_ui(this->node);
    saturation.attach_ui( this->node );
}

void ColoredConfig::add_listener( simparm::Listener& l ) {
    l.receive_changes_from( hue.value );
    l.receive_changes_from( saturation.value );
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
