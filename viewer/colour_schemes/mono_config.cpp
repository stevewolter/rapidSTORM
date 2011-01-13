#include "mono.h"
#include "mono_config.h"
#include "../Config.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

MonoConfig::MonoConfig() 
    : simparm::Object("BlackWhite", "Greyscale") {}

std::auto_ptr<Backend> MonoConfig::make_backend( Config& config, Status& status ) const
{
    return Backend::create< Mono >(Mono(config.invert()), config, status);
}

}

template <>
std::auto_ptr<ColourScheme> ColourScheme::config_for<colour_schemes::Mono>()
{
    return std::auto_ptr<ColourScheme>(new colour_schemes::MonoConfig());
}

}
}
