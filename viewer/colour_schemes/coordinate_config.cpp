#include "coordinate.h"
#include <viewer/ColourScheme.h>
#include <simparm/Object.h>
#include <simparm/Entry.h>
#include <dStorm/output/binning/config.h>

#include "../Config.h"

namespace dStorm {
namespace viewer {
namespace colour_schemes {

struct CoordinateConfig : public ColourScheme
{
    output::binning::FieldChoice choice;
    simparm::Entry<double> range;
    simparm::BaseAttribute::ConnectionStore listening;
    default_on_copy< boost::signals2::signal<void()> > change;

    CoordinateConfig();
    CoordinateConfig(const CoordinateConfig&);
    CoordinateConfig* clone() const { return new CoordinateConfig(*this); }
    std::auto_ptr<Backend> make_backend( Config&, Status& ) const;
    void add_listener( simparm::BaseAttribute::Listener );
    void attach_ui( simparm::NodeHandle );
};

CoordinateConfig::CoordinateConfig() 
: ColourScheme("ByCoordinate", "Vary hue with coordinate value"),
  choice("HueCoordinate", "Coordinate to vary hue with", output::binning::InteractivelyScaledToInterval, "Hue"),
  range("HueRange", "Range of hue", 0.666)
{
}

CoordinateConfig::CoordinateConfig(const CoordinateConfig& o) 
: ColourScheme(o), choice(o.choice), range(o.range)
{
}

void CoordinateConfig::add_listener( simparm::BaseAttribute::Listener l ) {
    choice.add_listener( l );
    change.connect( l );
}

void CoordinateConfig::attach_ui( simparm::NodeHandle at ) {
    listening = range.value.notify_on_value_change( change );
    simparm::NodeHandle r = attach_parent(at);
    choice.attach_ui( r );
    range.attach_ui( r );
}

std::auto_ptr<Backend> CoordinateConfig::make_backend( Config& config, Status& status ) const
{
    return Backend::create< Coordinate >(Coordinate(config.invert(), choice().make_user_scaled_binner(), range()), status);
}

}

template <>
std::auto_ptr<ColourScheme> ColourScheme::config_for<colour_schemes::Coordinate>()
{
    return std::auto_ptr<ColourScheme>(new colour_schemes::CoordinateConfig());
}

}
}
