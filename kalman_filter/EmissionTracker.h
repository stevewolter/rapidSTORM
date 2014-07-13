#ifndef DSTORM_KALMAN_FILTER_EMISSION_TRACKER_CONFIG_H
#define DSTORM_KALMAN_FILTER_EMISSION_TRACKER_CONFIG_H

#include <boost/units/quantity.hpp>

#include "kalman_filter/units.h"
#include "output/Output.h"
#include "output/OutputSource.h"
#include "simparm/BoostUnits.h"
#include "simparm/Entry.h"
#include "units/frame_count.h"

namespace dStorm {
namespace kalman_filter {
namespace emission_tracker {

class Config {
public:
    Config();
    void attach_ui( simparm::NodeHandle at );
    static std::string get_name() { return "EmissionTracker"; }
    static std::string get_description() { return "Track emissions"; }
    static simparm::UserLevel get_user_level() { return simparm::Beginner; }

    simparm::Entry< frame_count > allowBlinking;
    simparm::Entry< boost::units::quantity<diffusion_unit> > diffusion;
    simparm::Entry< boost::units::quantity<mobility_unit> > mobility;
    simparm::Entry<float> distance_threshold;
};

std::auto_ptr< dStorm::output::OutputSource > create();
std::unique_ptr<output::Output> create_default(
    const Config& config, std::unique_ptr<output::Output> suboutput);

}
}
}

#endif
