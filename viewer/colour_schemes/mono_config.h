#ifndef DSTORM_COLOUR_SCHEMES_Mono_CONFIG_H
#define DSTORM_COLOUR_SCHEMES_Mono_CONFIG_H

#include <viewer/ColourScheme.h>
#include <simparm/Object.hh>

namespace dStorm {
namespace viewer {
namespace colour_schemes {

struct MonoConfig : public ColourScheme, public simparm::Object
{
    MonoConfig();
    MonoConfig* clone() const { return new MonoConfig(*this); }
    simparm::Node& getNode() { return *this; }
    std::auto_ptr<Backend> make_backend( Config&, Status& ) const;
};

}
}
}

#endif
