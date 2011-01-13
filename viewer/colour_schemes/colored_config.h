#ifndef DSTORM_COLOUR_SCHEMES_COLORED_CONFIG_H
#define DSTORM_COLOUR_SCHEMES_COLORED_CONFIG_H

#include <viewer/ColourScheme.h>
#include <simparm/Object.hh>
#include <simparm/NumericEntry.hh>

namespace dStorm {
namespace viewer {
namespace colour_schemes {

struct ColoredConfig : public ColourScheme, public simparm::Object
{
    simparm::DoubleEntry hue, saturation;

    ColoredConfig();
    ColoredConfig(const ColoredConfig&);
    ColoredConfig* clone() const { return new ColoredConfig(*this); }
    simparm::Node& getNode() { return *this; }
    std::auto_ptr<Backend> make_backend( Config&, Status& ) const;
};

}
}
}

#endif
