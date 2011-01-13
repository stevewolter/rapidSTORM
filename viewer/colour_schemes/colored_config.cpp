#include "colored.h"
#include "colored_config.h"
#include "doc/help/context.h"
#include "../Config.h"
#include "base_impl.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

ColoredConfig::ColoredConfig() 
: simparm::Object("FixedHue", "Constant colour"),
  hue("Hue", "Select color hue", 0),
  saturation("Saturation", "Select saturation", 1)
{
    hue.helpID = HELP_Viewer_Hue;
    hue.setMin(0);
    hue.setMax(1);
    hue.setHelp("Select a hue between 0 and 1 to display localizations in."
                " The hue is selected along the HSV color axis, following "
                "the natural spectrum from 0 (red) over 1/6 (yellow), "
                "1/3 (green), 1/2 (cyan), 2/3 (blue) to 5/6 (violet) and "
                "1 (red again)");
    saturation.helpID = HELP_Viewer_Saturation;
    saturation.setMin(0);
    saturation.setMax(1);
    saturation.setHelp("Select a saturation between 0 and 1 for the color "
                       "in the display. Saturation 0 means no color (pure "
                       "black to pure white) and 1 means fully saturated "
                       "color.");

    push_back( hue );
    push_back( saturation );
}

ColoredConfig::ColoredConfig(const ColoredConfig& o)
: simparm::Object(o),
  hue(o.hue), saturation(o.saturation)
{
    push_back( hue );
    push_back( saturation );
}

std::auto_ptr<Backend> ColoredConfig::make_backend( Config& config, Status& status ) const
{
    return Backend::create< Colored >(Colored(config.invert(), hue(), saturation()), config, status);
}

}

template <>
std::auto_ptr<ColourScheme> ColourScheme::config_for<colour_schemes::Colored>()
{
    return std::auto_ptr<ColourScheme>(new colour_schemes::ColoredConfig());
}

}
}
