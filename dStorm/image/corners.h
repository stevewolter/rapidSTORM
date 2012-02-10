#ifndef DSTORM_IMAGE_CORNERS_H
#define DSTORM_IMAGE_CORNERS_H

#include <dStorm/Image.h>

namespace dStorm {

template <class Pixel, int Dim>
typename Image<Pixel,Dim>::Position
lower_corner( const Image<Pixel,Dim>& i ) {
    return Image<Pixel,Dim>::Position::Constant(0 * camera::pixel);
}

template <class Pixel, int Dim>
typename Image<Pixel,Dim>::Position
upper_corner( const Image<Pixel,Dim>& i ) {
    return i.sizes() - 1 * camera::pixel;
}

}

#endif
