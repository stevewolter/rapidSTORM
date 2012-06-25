#ifndef DSTORM_DENSITY_MAP_GAUSSIAN_SMOOTHING_H
#define DSTORM_DENSITY_MAP_GAUSSIAN_SMOOTHING_H

#include <memory>

namespace dStorm {
namespace density_map {

template <int Dim> class InterpolatorFactory;
template <int Dim> std::auto_ptr< InterpolatorFactory<Dim> > make_gaussian_smoothed_interpolator_factory();

}
}

#endif
