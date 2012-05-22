#include "ZTruthConfig.h"
#include "ZTruth.h"

namespace dStorm {
namespace calibrate_3d {

ZTruthConfig::ZTruthConfig()
: filter_("3DFilter", "Filter expression for usable spots"),
  new_z_("CalibratedZ", "Expression for true Z value")
{}

bool ZTruthConfig::has_z_truth() const {
    return new_z_() != "";
}
std::auto_ptr<ZTruth> ZTruthConfig::get_z_truth() const {
    if ( has_z_truth() )
        return std::auto_ptr<ZTruth>( new ZTruth( filter_(), new_z_() ) );
    else
        throw std::runtime_error("A Z calibration expression is necessary, but not given");
}

void ZTruthConfig::registerNamedEntries( simparm::Node& at ) {
    new_z_.attach_ui( at );
    filter_.attach_ui( at );
}

}
}
