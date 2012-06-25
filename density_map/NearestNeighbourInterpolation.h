#ifndef DSTORM_DENSITY_MAP_NEAREST_NEIGHBOUR_INTERPOLATION_H
#define DSTORM_DENSITY_MAP_NEAREST_NEIGHBOUR_INTERPOLATION_H

#include <memory>

namespace dStorm {
namespace density_map {

template <int Dim> class Interpolator;
template <int Dim> class InterpolatorFactory;

template <int Dim> std::auto_ptr< Interpolator<Dim> > make_nearest_neighbour_interpolator();
template <int Dim> std::auto_ptr< InterpolatorFactory<Dim> > make_nearest_neighbour_interpolator_factory();

}
}

#endif
