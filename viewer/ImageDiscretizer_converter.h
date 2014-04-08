#ifndef DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_CONVERTER_H
#define DSTORM_TRANSMISSIONS_IMAGEDISCRETIZER_CONVERTER_H

#include "viewer/ImageDiscretizer.h"
#include "density_map/Coordinates.h"

namespace dStorm {
namespace viewer {

template <typename ImageListener>
template <typename OtherListener>
Discretizer<ImageListener>
::Discretizer(
    const Discretizer<OtherListener>& o,
    const Image<float,Im::Dim>& binned_image,
    ColourScheme& colorizer) 
: total_pixel_count(o.total_pixel_count),
  colorizer(colorizer),
  max_value(o.max_value), 
  max_value_used_for_disc_factor(o.max_value_used_for_disc_factor),
  disc_factor(o.disc_factor),
  histogram(o.histogram),
  transition(o.transition),
  in_depth( o.in_depth ),
  out_depth( o.out_depth ),
  pixels_above_used_max_value( o.pixels_above_used_max_value ),
  histogram_power(o.histogram_power),
  cutoff_factor(o.cutoff_factor),
  binned_image(binned_image)
{
}

}
}

#endif
