#ifndef DSTORM_IMAGETRAITS_IMPL_H
#define DSTORM_IMAGETRAITS_IMPL_H

#include "ImageTraits.h"
#include <stdint.h>
#include <boost/units/systems/si/io.hpp>
#include <boost/units/io.hpp>
#include "units/nanolength.h"

namespace dStorm {
namespace input {

template <typename PixelType, int Dimensions>
template <typename OtherPixelType>
Traits< dStorm::Image<PixelType,Dimensions> >::Traits
    ( const Traits< dStorm::Image<OtherPixelType,Dimensions> >& o )
: input::BaseTraits(o), DataSetTraits(o), ImageTraits<Dimensions>(o), 
  traits::Optics<Dimensions>(o)
{}

#if 0
template <typename Pixel, int Dim>
void
Traits< dStorm::Image<Pixel,Dim> >::
compute_resolution( const Config& config ) {
    this->resolution = typename SizeTraits<Dim>::Resolution::value_type( 
        camera::pixels_per_meter /
            (config.pixel_size_in_nm() / 1E9) );
}
#endif

}
}

#endif
