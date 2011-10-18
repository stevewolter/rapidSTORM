#ifndef DSTORM_IMAGE_MIRROR_H
#define DSTORM_IMAGE_MIRROR_H

#include "../Image.h"
#include "constructors.h"

namespace dStorm {
namespace image {

template <typename PixelType, int Dimensions, int ToMirror>
Image<PixelType,Dimensions>
mirror( const Image<PixelType,Dimensions>& input )
{
    BOOST_STATIC_ASSERT(Dimensions > ToMirror);
    typename Image<PixelType,Dimensions>::Offsets o = input.get_offsets();
    size_t total_offset = input.get_global_offset();

    total_offset += (input.sizes()[ToMirror].value() - 1) * o[ToMirror];
    o[ToMirror] *= -1;
    return Image<PixelType,Dimensions>(input.sizes(), input.get_data_reference(), o, total_offset, input.frame_number());
}

}
}

#endif
