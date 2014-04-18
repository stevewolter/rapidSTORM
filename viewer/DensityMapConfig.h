#ifndef DSTORM_VIEWER_DENSITYMAPCONFIG_H
#define DSTORM_VIEWER_DENSITYMAPCONFIG_H

#include <simparm/Entry.h>
#include "density_map/CoordinatesFactory.h"
#include "density_map/InterpolatorChoice.h"
#include "viewer/Image.h"

namespace dStorm {
namespace viewer {

class DensityMapConfig {
public:
    typedef Eigen::Matrix< boost::units::quantity<boost::units::camera::length,int>, Im::Dim, 1 >
        CropBorder;

    density_map::CoordinatesFactory<Im::Dim> binned_dimensions;
    density_map::InterpolatorChoice<Im::Dim> interpolator;
    simparm::Entry< CropBorder > border;

    DensityMapConfig();
    void attach_ui( simparm::NodeHandle at );
    void backend_needs_changing( simparm::BaseAttribute::Listener );
    CropBorder crop_border() const;

private:
    dStorm::default_on_copy< boost::signals2::signal<void()> > backend_change;
    simparm::BaseAttribute::ConnectionStore listening;
};

}
}

#endif
