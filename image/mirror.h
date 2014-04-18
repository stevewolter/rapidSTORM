#ifndef DSTORM_IMAGE_MIRROR_H
#define DSTORM_IMAGE_MIRROR_H

#include "image/Image.h"
#include "image/constructors.h"

namespace dStorm {
namespace image {

template <typename PixelType, int Dimensions>
Image<PixelType,Dimensions>
mirror( const Image<PixelType,Dimensions>& input, int dimension )
{
    assert( dimension < Dimensions && dimension > 0 );
    typename Image<PixelType,Dimensions>::Offsets o = input.get_offsets();
    size_t total_offset = input.get_global_offset();

    total_offset += (input.sizes()[dimension].value() - 1) * o[dimension];
    o[dimension] *= -1;
    return Image<PixelType,Dimensions>(input.sizes(), input.get_data_reference(), o, total_offset, input.frame_number());
}

}
}

#endif
