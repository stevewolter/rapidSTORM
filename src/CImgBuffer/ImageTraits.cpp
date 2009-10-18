#include "ImageTraits.h"
#include "Config.h"
#include <stdint.h>

namespace CImgBuffer {

template <typename PixelType>
void
Traits< cimg_library::CImg<PixelType> >::
compute_resolution( const CImgBuffer::Config& config ) {
    for (int i = 0; i < resolution.rows(); i++)
        resolution[i] = config.pixel_size_in_nm() / 1E9;
}

template <typename PixelType>
void
Traits< cimg_library::CImg<PixelType> >::
set_resolution( const Eigen::Vector3d& resolution ) {
    for (int i = 0; i < resolution.rows(); i++)
        this->resolution[i] = resolution[i];
}

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
