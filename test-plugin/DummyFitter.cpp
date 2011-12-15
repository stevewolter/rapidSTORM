#define BOOST_DISABLE_ASSERTS
#include <dStorm/log.h>
#include "DummyFitter.h"
#include <dStorm/engine/JobInfo.h>
#include <dStorm/Localization.h>
#include <dStorm/engine/Spot.h>

namespace dStorm {
namespace debugplugin {

DummyFitterConfig::DummyFitterConfig()
: simparm::Set("DummyFitter", "Dummy fitter")
{
}

DummyFitterConfig::~DummyFitterConfig()
{
}

DummyFitter::DummyFitter(const Config&, const dStorm::engine::JobInfo& i) 
    : traits(i.traits), counter(0), length(5) {}

int DummyFitter::fitSpot( const dStorm::engine::Spot& spot, const dStorm::engine::Image &im, iterator target )
{
    if ( ++counter % length == 0 ) {
        DEBUG("Using result in image " << im.frame_number() << " at counter " << counter);
        Localization::Position::Type p;
        p.x() = spot.x() * camera::pixel / traits.plane(0).resolution(0)->in_dpm();
        p.y() = spot.y() * camera::pixel / traits.plane(0).resolution(1)->in_dpm();
        *target++ = dStorm::Localization( p, 1000 * camera::ad_counts );
        return 1;
    } else {
        return -1;
    }
}

void DummyFitterConfig::set_traits( output::Traits&, const engine::JobInfo& ) {
    /* TODO: Set traits */
}

}
}
