#ifndef DSTORM_CALIBRATE3D_ZTRUTHCONFIG_H
#define DSTORM_CALIBRATE3D_ZTRUTHCONFIG_H

#include <string>
#include <simparm/Entry.hh>
#include <memory>

namespace dStorm {
namespace calibrate_3d {

class ZTruth;

class ZTruthConfig {
    simparm::Entry<std::string> filter_, new_z_;
public:
    ZTruthConfig();
    void registerNamedEntries( simparm::Node& at );
    bool has_z_truth() const;
    std::auto_ptr<ZTruth> get_z_truth() const;
};

}
}

#endif
