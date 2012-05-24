#ifndef DSTORM_COLOUR_SCHEMES_Mono_CONFIG_H
#define DSTORM_COLOUR_SCHEMES_Mono_CONFIG_H

#include <viewer/ColourScheme.h>
#include <simparm/Object.h>

namespace dStorm {
namespace viewer {
namespace colour_schemes {

struct MonoConfig : public ColourScheme
{
    MonoConfig();
    MonoConfig* clone() const { return new MonoConfig(*this); }
    std::auto_ptr<Backend> make_backend( Config&, Status& ) const;
    void attach_ui( simparm::NodeHandle at ) { attach_parent(at); }
    void add_listener( simparm::BaseAttribute::Listener ) {}
};

}
}
}

#endif
