#ifndef DSTORM_DISPLAYDATASOURCE_H
#define DSTORM_DISPLAYDATASOURCE_H

#include "DisplayDataSource.h"

namespace dStorm {
namespace Display {

template <class PixelType>
void Change::display_normalized(
    const dStorm::Image<PixelType,2>& i
) 
{
    typename dStorm::Image<PixelType,2>::PixelPair p = i.minmax();
    do_resize = true;
    resize_image.size = i.sizes();
    resize_image.key_size = 256;
    do_change_image = true;
    image_change.new_image = i.template normalize<uint8_t>(p).template convert<dStorm::Pixel>();
    make_linear_key( p );
}

}
}

#endif
