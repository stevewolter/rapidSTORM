#ifndef DSTORM_IMAGE_CONSTRUCTORS_H
#define DSTORM_IMAGE_CONSTRUCTORS_H

#include "../Image.h"
#include "iterator.h"
#include <boost/type_traits/has_trivial_copy.hpp>

namespace dStorm {

template <typename PixelType, int Dimensions>
Image<PixelType,Dimensions>::~Image() 
{
}

template <typename PixelType, int Dimensions>
Image<PixelType,Dimensions>::Image() {}

template <typename PixelType, int Dimensions>
Image<PixelType,Dimensions>::Image(
    Size sz, frame_index i
)
: sz(sz), fn(i)
{
    offsets[0] = 1;
    for (int i = 1; i < Dimensions; ++i) {
        offsets[i] = offsets[i-1] * sz[i-1].value();
    }
    this->img = boost::shared_array<PixelType>( new PixelType[offsets[Dimensions-1] * sz[Dimensions-1].value()] );
    global_offset = 0;
}

template <typename PixelType, int Dimensions>
Image<PixelType,Dimensions>::Image(
    Size sz, boost::shared_array<PixelType> data, Offsets o, size_t global_offset, frame_index i
) : img(data), sz(sz), offsets(o), global_offset(global_offset), fn(i)
{}

template <typename PixelType, int Dimensions>
Image<PixelType,Dimensions>
Image<PixelType,Dimensions>::deep_copy() const 
{
    Image image( this->sz, this->fn );
    if ( false /* image is not guaranteed to be contiguous */ && boost::has_trivial_copy<PixelType>::value )
        memcpy( image.img.get(), this->img.get(), this->size_in_pixels() );
    else
        std::copy( this->begin(), this->end(), image.begin() );
    return image;
}

template <typename PixelType, int Dimensions>
void Image<PixelType,Dimensions>::fill(PixelType type)
{
    for ( iterator i = begin(); i != end(); ++i ) *i = type;
}

}

#endif
