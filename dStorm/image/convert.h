#ifndef DSTORM_IMAGE_CONVERT_H
#define DSTORM_IMAGE_CONVERT_H

#include "../Image.h"
#include "iterator.h"
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
    typename Image<PixelType,Dimensions>::const_iterator i = this->begin(), e = this->end();
    typename Image<ReducedPixel,Dimensions>::iterator o = rv.begin();
    for ( ; i != e; ++i, ++o) {
        PixelType cropped = 
            std::max( minmax.first, std::min( *i, minmax.second ) );
        *o = (cropped - minmax.first) * nf
            + std::numeric_limits<ReducedPixel>::min();
    }

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
    std::copy( this->begin(), this->end(), rv.begin() );
    return rv;
}

}

#endif
