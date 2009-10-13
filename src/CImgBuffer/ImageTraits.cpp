#include "ImageTraits.h"
#include "Config.h"
#include <stdint.h>

namespace CImgBuffer {

template <typename PixelType>
void
Traits< cimg_library::CImg<PixelType> >::
compute_resolution( const CImgBuffer::Config& config ) {
    /* dots per nanometre is inverse of pixel size, and
     * dots per inch is 2.54 (number of cm per inch) times
     * 10^7 (number of nm per cm) times dots per nanometre */
    xr = yr = zr = 2.54E7 / config.pixel_size_in_nm();
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
