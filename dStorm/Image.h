#ifndef DSTORM_IMAGE_H
#define DSTORM_IMAGE_H

#include "Image_decl.h"
#include "BaseImage.h"
#include <dStorm/units/camera_response.h>

namespace dStorm {

template <typename PixelType, int Dimensions>
class Image
: public BaseImage<PixelType,Dimensions> 
{
    typedef BaseImage<PixelType,Dimensions> Base;
  public:
    typedef typename Base::Size Size;
    typedef std::pair<PixelType,PixelType> PixelPair;

    Image();
    Image(Size sz);
    Image(Size sz, frame_index i);
    Image(Size sz, boost::shared_array<PixelType> data,
          frame_index i);
    ~Image();

    void fill(PixelType type);
    PixelPair minmax() const;

    template <typename ReducedPixel>
    Image<ReducedPixel,Dimensions> normalize() const;
    template <typename ReducedPixel>
    Image<ReducedPixel,Dimensions> convert() const;
    template <typename ReducedPixel>
    Image<ReducedPixel,Dimensions> 
    normalize( const PixelPair& minmax ) const;

    Image<bool,Dimensions> threshold( PixelType threshold ) const;

    Image deep_copy() const;
};

}

#endif
