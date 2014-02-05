#ifndef DSTORM_IMAGE_CROP_H
#define DSTORM_IMAGE_CROP_H

#include "dStorm/Image.h"

namespace dStorm {

template <typename PixelType, int Dimensions>
Image<PixelType,Dimensions>
crop( 
    const Image<PixelType,Dimensions>& image,  
    const typename Image<PixelType,Dimensions>::Size& lower_bound,
    const typename Image<PixelType,Dimensions>::Size& upper_bound
) {
    size_t new_offset = image.get_global_offset();
    typename Image<PixelType,Dimensions>::Size new_size;
    for (int i = 0; i < Dimensions; ++i) {
        new_offset += image.get_offsets()[i] * lower_bound[i].value();
        new_size[i] = std::min( image.sizes()[i] - 1 * camera::pixel, upper_bound[i] ) - lower_bound[i] 
            + 1 * boost::units::camera::pixel;
    }
    return Image<PixelType,Dimensions>(
        new_size,
        image.get_data_reference(), 
        image.get_offsets(),
        new_offset,
        image.frame_number() );
}

}

#endif
