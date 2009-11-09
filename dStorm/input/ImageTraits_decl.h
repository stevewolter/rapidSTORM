#include "Traits.h"

namespace cimg_library {
    template <typename Pixel> class CImg;
}

namespace dStorm {
namespace input {

template <typename PixelType>
class Traits< cimg_library::CImg<PixelType> >;

#if 0 /* Enable when C++0x arrives */
extern template class Traits< cimg_library::CImg<uint8_t> >;
extern template class Traits< cimg_library::CImg<uint16_t> >;
extern template class Traits< cimg_library::CImg<uint32_t> >;
extern template class Traits< cimg_library::CImg<int8_t> >;
extern template class Traits< cimg_library::CImg<int16_t> >;
extern template class Traits< cimg_library::CImg<int32_t> >;
extern template class Traits< cimg_library::CImg<float> >;
extern template class Traits< cimg_library::CImg<double> >;
extern template class Traits< cimg_library::CImg<long double> >;
#endif
    
}
}
