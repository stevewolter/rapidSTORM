#ifndef DSTORM_VIEWER_DENSITYMAPCONFIG_H
#define DSTORM_VIEWER_DENSITYMAPCONFIG_H

#include "Config_decl.h"
#include <simparm/Entry.h>
#include <dStorm/outputs/BinnedLocalizations_strategies_config.h>
#include "density_map/InterpolatorChoice.h"
#include "Image.h"

namespace dStorm {
namespace viewer {

class DensityMapConfig {
public:
    typedef Eigen::Matrix< boost::units::quantity<boost::units::camera::length,int>, 3, 1 >
        CropBorder;

    outputs::DimensionSelector<Im::Dim> binned_dimensions;
    density_map::InterpolatorChoice<3> interpolator;
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
