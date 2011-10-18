#ifndef DSTORM_IMAGE_MINMAX_H
#define DSTORM_IMAGE_MINMAX_H

#include "../Image.h"
#include "iterator.h"
#include <algorithm>

namespace dStorm {

template <typename PixelType, int Dimensions>
typename Image<PixelType,Dimensions>::PixelPair
Image<PixelType,Dimensions>::minmax() const {
    typename Image<PixelType,Dimensions>::const_iterator i = this->begin(), e = this->end();
    PixelPair p;
    p.first = *i;
    p.second = *i;

    for ( ++i; i != e; ++i ) { 
        p.first = std::min( p.first, *i );
        p.second = std::max( p.second, *i );
    }

    return p;
}

template <typename PixelType, int Dimensions>
Image<bool,Dimensions>
Image<PixelType,Dimensions>::threshold(PixelType t) const 
{
    Image<bool,Dimensions> rv(this->sz, this->fn);
    std::transform( this->begin(), this->end(), rv.begin(), std::bind1st( std::less<PixelType>(), t ) );
    return rv;
}

}

#endif
