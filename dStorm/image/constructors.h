#ifndef DSTORM_IMAGE_CONSTRUCTORS_H
#define DSTORM_IMAGE_CONSTRUCTORS_H

#include <dStorm/Image.h>

namespace dStorm {

template <typename PixelType, int Dimensions>
BaseImage<PixelType,Dimensions>::~BaseImage() 
{
}

template <typename PixelType, int Dimensions>
BaseImage<PixelType,Dimensions>::BaseImage()
: _pxc(0) {}

template <typename PixelType, int Dimensions>
BaseImage<PixelType,Dimensions>::BaseImage(
    Size sz, frame_index i
)
: sz(sz), fn(i), _pxc(1) 
{
    for (int i = 0; i < Dimensions; i++)
        _pxc *= sz[i] / cs_units::camera::pixel;
    this->img = boost::shared_array<PixelType>( new PixelType[_pxc] );
}

template <typename PixelType, int Dimensions>
Image<PixelType,Dimensions>::Image() {}

template <typename PixelType, int Dimensions>
Image<PixelType,Dimensions>::Image(Size sz)
: Base(sz, 0 * cs_units::camera::frame) {}

template <typename PixelType, int Dimensions>
Image<PixelType,Dimensions>::Image(Size sz, frame_index i)
: Base(sz, i) {}


template <typename PixelType, int Dimensions>
void Image<PixelType,Dimensions>::fill(PixelType type)
{
    for (unsigned long i = 0; i < this->_pxc; i++)
        this->img[i] = type;
}

template <typename PixelType, int Dimensions>
Image<PixelType,Dimensions>::~Image() {}

}

#endif
