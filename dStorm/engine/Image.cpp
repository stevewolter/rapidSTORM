#include "Image.h"
#include <dStorm/image/Image.hpp>
#include <dStorm/image/MetaInfo.h>
#include <dStorm/engine/InputTraits.h>

namespace dStorm {

//template class BaseImage<engine::StormPixel,2>;
//template class BaseImage<engine::SmoothedPixel,2>;
//template class Image<engine::StormPixel,2>;
//template class Image<engine::SmoothedPixel,2>;

template Image< dStorm::engine::SmoothedPixel, 2 >::~Image();
template void Image< dStorm::engine::SmoothedPixel, 2 >::fill( engine::SmoothedPixel );

namespace input {

template class Traits< dStorm::engine::ImageStack >;
    
}
}
