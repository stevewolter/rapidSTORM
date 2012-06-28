#ifndef DSTORM_COLOUR_SCHEMES_HOT_CONFIG_H
#define DSTORM_COLOUR_SCHEMES_HOT_CONFIG_H

#include <viewer/ColourSchemeFactory.h>
#include <simparm/Object.h>

namespace dStorm {
namespace viewer {
namespace colour_schemes {

struct HotConfig : public ColourSchemeFactory
{
    HotConfig();
    HotConfig* clone() const { return new HotConfig(*this); }
    std::auto_ptr<ColourScheme> make_backend( bool invert ) const;
    void attach_ui( simparm::NodeHandle at ) { attach_parent(at); }
    void add_listener( simparm::BaseAttribute::Listener ) {}
};

}
}
}

#endif
