#ifndef DSTORM_IMAGE_SLICE_H
#define DSTORM_IMAGE_SLICE_H

#include "dStorm/image/Image.h"
#include <cassert>

namespace dStorm {

template <typename PixelType, int Dimensions>
Image<PixelType,Dimensions-1> Image<PixelType,Dimensions>::slice( int dimension, typename Size::Scalar layer ) const
{
    assert( dimension < Dimensions );

    Image<PixelType,Dimensions-1> rv;
    rv.fn = this->fn; rv.bg_sigma = this->bg_sigma; rv.img = this->img;

    for (int i = 0, j = 0; i < Dimensions; ++i) {
        if ( i != dimension )  {
            rv.sz[j] = this->sz[i];
            rv.offsets[j] = this->offsets[i];
            ++j;
        } else {
            assert( layer < this->sz[i] );
            rv.global_offset = this->global_offset + layer.value() * this->offsets[i];
        }
    }

    return rv;
}

}

#endif
