#ifndef DSTORM_COLOUR_SCHEMES_HOT_CONFIG_H
#define DSTORM_COLOUR_SCHEMES_HOT_CONFIG_H

#include <viewer/ColourScheme.h>
#include <simparm/Object.hh>

namespace dStorm {
namespace viewer {
namespace colour_schemes {

struct HotConfig : public ColourScheme
{
    HotConfig();
    HotConfig* clone() const { return new HotConfig(*this); }
    std::auto_ptr<Backend> make_backend( Config&, Status& ) const;
    void attach_ui( simparm::NodeHandle at ) { attach_parent(at); }
    void add_listener( simparm::BaseAttribute::Listener ) {}
};

}
}
}

#endif
