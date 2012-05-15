#ifndef DSTORM_COLOUR_SCHEMES_COLORED_CONFIG_H
#define DSTORM_COLOUR_SCHEMES_COLORED_CONFIG_H

#include <viewer/ColourScheme.h>
#include <simparm/Object.hh>
#include <simparm/Entry.hh>

namespace dStorm {
namespace viewer {
namespace colour_schemes {

struct ColoredConfig : public ColourScheme
{
    simparm::Entry<double> hue, saturation;

    ColoredConfig();
    ColoredConfig(const ColoredConfig&);
    ColoredConfig* clone() const { return new ColoredConfig(*this); }
    std::auto_ptr<Backend> make_backend( Config&, Status& ) const;
    void add_listener( simparm::Listener& );
    void attach_ui( simparm::Node& );
};

}
}
}

#endif
