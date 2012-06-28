#include "DensityMapConfig.h"

namespace dStorm {
namespace viewer {

DensityMapConfig::DensityMapConfig() :
  border("Border", "Width of chopped border", CropBorder::Constant(0 * camera::pixel))
{
    border.set_user_level(simparm::Intermediate);
}

void DensityMapConfig::attach_ui( simparm::NodeHandle n ) {
    listening = border.value.notify_on_value_change( boost::ref(backend_change) );
    binned_dimensions.attach_ui(n);
    interpolator.attach_ui(n);
    border.attach_ui(n);
}

void DensityMapConfig::backend_needs_changing( simparm::BaseAttribute::Listener l ) {
    backend_change.connect( l );
    binned_dimensions.add_listener(l);
}

DensityMapConfig::CropBorder
DensityMapConfig::crop_border() const
{
    CropBorder rv = border();
    if ( ! binned_dimensions.is_3d() )
        rv[2] = 0 * camera::pixel;
    return rv;
}


}
}
