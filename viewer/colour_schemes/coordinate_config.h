#ifndef DSTORM_COLOUR_SCHEMES_COORDINATE_CONFIG_H
#define DSTORM_COLOUR_SCHEMES_COOORDINATE_CONFIG_H

#include <viewer/ColourScheme.h>
#include <simparm/Object.hh>
#include <simparm/Entry.hh>
#include <dStorm/output/binning/config.h>

namespace dStorm {
namespace viewer {
namespace colour_schemes {

struct CoordinateConfig : public ColourScheme
{
    output::binning::FieldChoice choice;
    simparm::Entry<double> range;

    CoordinateConfig();
    CoordinateConfig(const CoordinateConfig&);
    CoordinateConfig* clone() const { return new CoordinateConfig(*this); }
    std::auto_ptr<Backend> make_backend( Config&, Status& ) const;
    void add_listener( simparm::Listener& );
    void attach_ui( simparm::Node& );
};

}
}
}

#endif
