#include "dStorm/engine/Image.h"
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

namespace engine {

void ImageStack::push_back( const Plane& p ) 
{
    planes_.push_back( p );
}

void ImageStack::clear() { planes_.clear(); }

ImageStack::ImageStack() : fn( 0 * camera::frame ) {}
ImageStack::ImageStack( frame_index i) : fn(i) {}
ImageStack::ImageStack( const Image2D& p ) {
    fn = p.frame_number();
    planes_.push_back(p);
}

bool ImageStack::has_invalid_planes() const
{
    for (int i = 0; i < plane_count(); ++i)
        if ( plane(i).is_invalid() )
            return true;
    return false;
}

}

}
