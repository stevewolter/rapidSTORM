#include "ImageTraits.h"
#include "Config.h"
#include <stdint.h>
#include <iostream>
#include <boost/units/io.hpp>

namespace dStorm {
namespace input {

template <typename PixelType>
void
Traits< cimg_library::CImg<PixelType> >::
compute_resolution( const Config& config ) {
    resolution = Resolution::value_type( 
        cs_units::camera::pixels_per_meter /
            (config.pixel_size_in_nm() / 1E9) );
}

#if 0
template <typename PixelType>
void
Traits< cimg_library::CImg<PixelType> >::
set_resolution( const Eigen::Vector3d& resolution ) {
    for (int i = 0; i < resolution.rows(); i++)
        this->resolution[i] = resolution[i];
}
#endif

template class Traits< cimg_library::CImg<uint8_t> >;
template class Traits< cimg_library::CImg<uint16_t> >;
template class Traits< cimg_library::CImg<uint32_t> >;
template class Traits< cimg_library::CImg<int8_t> >;
template class Traits< cimg_library::CImg<int16_t> >;
template class Traits< cimg_library::CImg<int32_t> >;
template class Traits< cimg_library::CImg<float> >;
template class Traits< cimg_library::CImg<double> >;
template class Traits< cimg_library::CImg<long double> >;
    
}
}
