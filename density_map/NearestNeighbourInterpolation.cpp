#include "NearestNeighbourInterpolation.hpp"

namespace dStorm {
namespace density_map {

template std::auto_ptr< Interpolator<2> > make_nearest_neighbour_interpolator<2>();
template std::auto_ptr< Interpolator<3> > make_nearest_neighbour_interpolator<3>();
template std::auto_ptr< InterpolatorFactory<2> > make_nearest_neighbour_interpolator_factory<2>();
template std::auto_ptr< InterpolatorFactory<3> > make_nearest_neighbour_interpolator_factory<3>();

}
}
