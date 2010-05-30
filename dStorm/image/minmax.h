#ifndef DSTORM_IMAGE_MINMAX_H
#define DSTORM_IMAGE_MINMAX_H

#include <dStorm/Image.h>

namespace dStorm {

template <typename PixelType, int Dimensions>
typename Image<PixelType,Dimensions>::PixelPair
Image<PixelType,Dimensions>::minmax() const {
    PixelPair p;
    p.first = this->img[0];
    p.second = this->img[0];

    for (unsigned long i = 1; i < this->_pxc; i++) {
        p.first = std::min( p.first, this->img[i] );
        p.second = std::max( p.second, this->img[i] );
    }

    return p;
}

template <typename PixelType, int Dimensions>
Image<bool,Dimensions>
Image<PixelType,Dimensions>::threshold(PixelType t) const 
{
    Image<bool,Dimensions> rv(this->sz, this->fn);
    for (unsigned long i = 0; i < this->_pxc; i++)
        rv.ptr()[i] = (this->img[i] > t);
    return rv;
}

}

#endif
