#ifndef DSTORM_IMAGE_FIND_BY_OFFSET_H
#define DSTORM_IMAGE_FIND_BY_OFFSET_H

namespace dStorm {

template <typename PixelType, int Dimensions>
typename ImageTypes<Dimensions>::Position
find_by_offset( const Image<PixelType,Dimensions>& im, unsigned offset )
{
    typename ImageTypes<Dimensions>::Position rv;
    for (int i = Dimensions-1; i >= 0; --i) {
        assert( i == Dimensions-1 || im.get_offsets()[i] <= im.get_offsets()[i+1] );
        rv[i] = (offset / im.get_offsets()[i]) * camera::pixel;
        offset %= unsigned(im.get_offsets()[i]);
    }
    return rv;
}

}

#endif
