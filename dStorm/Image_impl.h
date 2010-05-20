#ifndef DSTORM_IMAGE_IMPL_H
#define DSTORM_IMAGE_IMPL_H
#include "Image.h"
#include <limits>

namespace dStorm {

template <typename PixelType, int Dimensions>
BaseImage<PixelType,Dimensions>::BaseImage()
: _pxc(0) {}

template <typename PixelType, int Dimensions>
BaseImage<PixelType,Dimensions>::BaseImage(
    Size sz, frame_index i
)
: sz(sz), fn(i), _pxc(1) {
    for (int i = 0; i < Dimensions; i++)
        _pxc *= sz[i] / cs_units::camera::pixel;
}

template <typename PixelType, int Dimensions>
void Image<PixelType,Dimensions>::fill(PixelType type)
{
    for (unsigned long i = 0; i < this->_pxc; i++)
        this->img[i] = type;
}

template <typename PixelType, int Dimensions>
typename Image<PixelType,Dimensions>::PixelPair
Image<PixelType,Dimensions>::minmax() const {
    PixelPair p;
    p.first = this->img[0];
    p.second = this->img[0];

    for (unsigned long i = 1; i < this->_pxc; i++) {
        p.first = std::min( p.first, this->img[i] );
        p.second = std::max( p.first, this->img[i] );
    }

    return p;
}

template <typename PixelType, int Dimensions>
template <typename ReducedPixel>
Image<ReducedPixel,Dimensions> 
Image<PixelType,Dimensions>::normalize(
    const PixelPair& minmax
) const {
    float nf = 
        (std::numeric_limits<ReducedPixel>::max() -
         std::numeric_limits<ReducedPixel>::min()) * 1.0f;
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
