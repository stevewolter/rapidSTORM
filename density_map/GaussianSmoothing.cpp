#include "GaussianSmoothing.hpp"

namespace dStorm {
namespace density_map {

template std::auto_ptr< InterpolatorFactory<2> > make_gaussian_smoothed_interpolator_factory<2>();
template std::auto_ptr< InterpolatorFactory<3> > make_gaussian_smoothed_interpolator_factory<3>();

}
}
