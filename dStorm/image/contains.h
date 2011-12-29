#ifndef DSTORM_IMAGE_CONTAINS_H
#define DSTORM_IMAGE_CONTAINS_H

namespace dStorm {

template <typename PixelType, int Dimensions>
bool contains( const Image< PixelType,Dimensions >& im, const typename ImageTypes<Dimensions>::Position& pos )
{
    return (im.sizes_in_pixels().array() > pos.array()).all();
}

}

#endif
