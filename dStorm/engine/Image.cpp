#include "Image.h"
#include "../Image_impl.h"
#include "../ImageTraits_impl.h"

namespace dStorm {

//template class BaseImage<engine::StormPixel,2>;
//template class BaseImage<engine::SmoothedPixel,2>;
//template class Image<engine::StormPixel,2>;
//template class Image<engine::SmoothedPixel,2>;

template Image< dStorm::engine::SmoothedPixel, 2 >::~Image();
template void Image< dStorm::engine::SmoothedPixel, 2 >::fill( engine::SmoothedPixel );

namespace input {

template class Traits< dStorm::engine::Image >;
template class Traits< dStorm::engine::SmoothedImage >;
    
}
}
