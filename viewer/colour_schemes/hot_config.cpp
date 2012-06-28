#include "hot.h"
#include "hot_config.h"
#include "../Config.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

HotConfig::HotConfig() 
    : ColourSchemeFactory("BlackRedYellowWhite", "Colour code ranging from red over yellow to white") {}

std::auto_ptr<Base> HotConfig::make_backend( bool invert ) const
{
    return std::auto_ptr<Base>(new Hot(invert));
}

std::auto_ptr<ColourSchemeFactory> make_hot_factory()
{
    return std::auto_ptr<ColourSchemeFactory>(new colour_schemes::HotConfig());
}

}
}
}
