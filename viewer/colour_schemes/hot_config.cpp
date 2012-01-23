#include "hot.h"
#include "hot_config.h"
#include "../Config.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

HotConfig::HotConfig() 
    : simparm::Object("BlackRedYellowWhite", "Colour code ranging from red over yellow to white") {}

std::auto_ptr<Backend> HotConfig::make_backend( Config& config, Status& status ) const
{
    return Backend::create< Hot >(Hot(config.invert()), status);
}

}

template <>
std::auto_ptr<ColourScheme> ColourScheme::config_for<colour_schemes::Hot>()
{
    return std::auto_ptr<ColourScheme>(new colour_schemes::HotConfig());
}

}
}
