#ifndef DSTORM_IMAGE_MINMAX_H
#define DSTORM_IMAGE_MINMAX_H

#include "image/Image.h"
#include "image/iterator.h"
#include <algorithm>

namespace dStorm {

template <typename PixelType, int Dimensions>
Image<PixelType,Dimensions>
operator-(const Image<PixelType,Dimensions>& a, const Image<PixelType,Dimensions>& b) {
    return subtract(a, b);
}

template <typename PixelType, int Dimensions>
Image<PixelType,Dimensions>
subtract(const Image<PixelType,Dimensions>& a, const Image<PixelType,Dimensions>& b) {
    assert(a.sizes() == b.sizes());
    Image<PixelType,Dimensions> result = a.deep_copy();

    for (auto i = a.begin(), j = b.begin(), k = result.begin(); i != a.end(); ++i, ++j, ++k) {
        assert(j != b.end());
        assert(k != result.end());
        if (boost::is_unsigned<PixelType>::value && *i < *j) {
            *k = 0;
        } else {
            *k = *i - *j;
        }
    }

    return result;
}

}

#endif
