#include "Image.h"
#include <dStorm/Image_impl.h>
#include <dStorm/ImageTraits_impl.h>

namespace dStorm {

template class BaseImage<engine::StormPixel,2>;
template class BaseImage<engine::SmoothedPixel,2>;
template class Image<engine::StormPixel,2>;
template class Image<engine::SmoothedPixel,2>;

namespace input {

template class Traits< dStorm::engine::Image >;
template class Traits< dStorm::engine::SmoothedImage >;
    
}
}
