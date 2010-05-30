#ifndef DSTORM_IMAGE_CONVERT_H
#define DSTORM_IMAGE_CONVERT_H

#include <dStorm/Image.h>
#include <limits>

namespace dStorm {

template <typename PixelType, int Dimensions>
template <typename ReducedPixel>
Image<ReducedPixel,Dimensions> 
Image<PixelType,Dimensions>::normalize(
    const PixelPair& minmax
) const {
    float nf = 
        (std::numeric_limits<ReducedPixel>::max() -
         std::numeric_limits<ReducedPixel>::min()) * 1.0f;
    if ( minmax.second == minmax.first )
        nf = 0;
    else
        nf /= (minmax.second - minmax.first);

    Image<ReducedPixel,Dimensions> rv(this->sz, this->fn);
    for (unsigned long i = 0; i < this->_pxc; i++)
        rv.ptr()[i] = (this->img[i] - minmax.first) * nf
            + std::numeric_limits<ReducedPixel>::min();

    return rv;
}

template <typename PixelType, int Dimensions>
template <typename ReducedPixel>
Image<ReducedPixel,Dimensions> 
Image<PixelType,Dimensions>::normalize() const {
    PixelPair p = this->minmax();
    return this->normalize<ReducedPixel>(p);
}

template <typename PixelType, int Dimensions>
template <typename ReducedPixel>
Image<ReducedPixel,Dimensions> 
Image<PixelType,Dimensions>::convert() const
{
    Image<ReducedPixel,Dimensions> rv(this->sz, this->fn);
    for (unsigned long i = 0; i < this->_pxc; i++)
        rv.ptr()[i] = this->img[i];
    return rv;
}

}

#endif
