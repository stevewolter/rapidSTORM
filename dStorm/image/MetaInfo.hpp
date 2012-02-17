#ifndef DSTORM_IMAGETRAITS_IMPL_H
#define DSTORM_IMAGETRAITS_IMPL_H

#include "MetaInfo.h"
#include <stdint.h>
#include <boost/units/systems/si/io.hpp>
#include <boost/units/io.hpp>
#include <dStorm/units/nanolength.h>
#include <dStorm/traits/Projection.h>

namespace dStorm {
namespace image {

template <int Dimensions>
MetaInfo< Dimensions >::MetaInfo() {}

template <int Dimensions>
traits::ImageResolution MetaInfo<Dimensions>::resolution(int r) const {
    assert( resolutions_[r].is_initialized() );
    return *resolutions_[r];
}

template <int Dimensions>
bool MetaInfo<Dimensions>::has_resolution() const {
    bool all_set = true;
    for (int i = 0; i < Dimensions; ++i)
        all_set = all_set && resolutions_[i];
    return all_set;
}

template <int Dimensions>
const typename MetaInfo<Dimensions>::Resolutions& 
MetaInfo<Dimensions>::image_resolutions() const {
    return resolutions_;
}

template <int Dimensions>
void MetaInfo<Dimensions>::set_resolution( const Resolutions& f )
{
    resolutions_ = f;
}


}
}

#endif
