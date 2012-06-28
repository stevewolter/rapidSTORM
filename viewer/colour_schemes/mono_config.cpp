#include "mono.h"
#include "mono_config.h"
#include "../Config.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

MonoConfig::MonoConfig() 
    : ColourSchemeFactory("BlackWhite", "Greyscale") {}

std::auto_ptr<Base> MonoConfig::make_backend( bool invert) const
{
    return std::auto_ptr<Base>(new Mono(invert));
}

std::auto_ptr<ColourSchemeFactory> make_mono_factory()
{
    return std::auto_ptr<ColourSchemeFactory>(new colour_schemes::MonoConfig());
}

}
}
}
