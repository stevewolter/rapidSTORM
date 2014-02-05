#ifndef DSTORM_CALIBRATE_3D_CONFIG_H
#define DSTORM_CALIBRATE_3D_CONFIG_H

#include <simparm/Eigen_decl.h>
#include <simparm/BoostUnits.h>
#include <simparm/Eigen.h>

#include <simparm/Object.h>
#include <simparm/Entry.h>
#include <string>
#include "output/Capabilities.h"
#include "calibrate_3d/FormCalibrationConfig.h"
#include "calibrate_3d/ZTruthConfig.h"
#include "units/microlength.h"
#include <boost/units/quantity.hpp>

namespace dStorm {
namespace calibrate_3d {

using boost::units::quantity;
namespace si = boost::units::si;

class Config : public FormCalibrationConfig, public ZTruthConfig {
    simparm::Entry<double> target_volume_;
    simparm::Entry<unsigned int> target_localization_number_;
    simparm::Entry< quantity<si::microlength> > missing_penalty_;
    simparm::Entry<double> relative_initial_step_, absolute_initial_step_;
public:
    Config();
    void attach_ui( simparm::NodeHandle );
    bool can_work_with(output::Capabilities cap) 
        { return cap.test( output::Capabilities::SourceImage ); }
    static std::string get_name() { return "Calibrate3D"; }
    static std::string get_description() { return "Calibrate 3D on known data"; }
    static simparm::UserLevel get_user_level() { return simparm::Expert; }

    double target_volume() const { return target_volume_(); }
    double relative_initial_step() const { return relative_initial_step_(); }
    double absolute_initial_step() const { return absolute_initial_step_(); }

    int target_localization_number() const { return target_localization_number_(); }
    quantity<si::microlength> missing_penalty() const { return missing_penalty_(); }
};

}
}

#endif
