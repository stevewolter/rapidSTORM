#ifndef DSTORM_DISPLAYDATASOURCE_H
#define DSTORM_DISPLAYDATASOURCE_H

#include "DataSource.h"

namespace dStorm {
namespace display {

template <class PixelType, int Dimensions>
void display_normalized(
    Change& result,
    const dStorm::Image<PixelType,Dimensions>& i
) 
{
    BOOST_STATIC_ASSERT(( Dimensions == Image::Dim ));
    typename dStorm::Image<PixelType,2>::PixelPair p = i.minmax();
    result.do_resize = true;
    result.resize_image.size = i.sizes();
    result.resize_image.keys.clear();
    result.resize_image.keys.push_back(
        KeyDeclaration("ADC", "A/D counts per pixel", 256) );
    result.do_change_image = true;
    result.image_change.new_image = i.template normalize<uint8_t>(p).template convert<dStorm::Pixel>();
    result.make_linear_key( p );
}

}
}

#endif
