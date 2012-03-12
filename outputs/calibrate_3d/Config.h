#ifndef DSTORM_CALIBRATE_3D_CONFIG_H
#define DSTORM_CALIBRATE_3D_CONFIG_H

#include <simparm/Object.hh>
#include <simparm/Entry.hh>
#include <string>
#include <dStorm/output/Capabilities.h>
#include "guf/form_fitter/Config.h"

namespace dStorm {
namespace calibrate_3d {

class Config_ : public simparm::Object, public form_fitter::FormCalibratorConfig {
    simparm::Entry<std::string> filter_, new_z_;
    simparm::Entry<double> target_volume_;
public:
    Config_();
    void registerNamedEntries();
    bool can_work_with(output::Capabilities cap) 
        { return cap.test( output::Capabilities::SourceImage ); }

    std::string filter() const { return filter_(); }
    std::string new_z() const { return new_z_(); }
    double target_volume() const { return target_volume_(); }
};

}
}

#endif
