#ifndef DSTORM_DENSITY_MAP_LINEARINTERPOLATION_H
#define DSTORM_DENSITY_MAP_LINEARINTERPOLATION_H

#include <memory>

namespace dStorm {
namespace density_map {

template <int Dim> class Interpolator;
template <int Dim> class InterpolatorFactory;

template <int Dim> std::auto_ptr< Interpolator<Dim> > make_linear_interpolator();
template <int Dim> std::auto_ptr< InterpolatorFactory<Dim> > make_linear_interpolator_factory();

}
}

#endif
