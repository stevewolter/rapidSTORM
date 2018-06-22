#include "debug.h"
#include "image/MetaInfo.hpp"
#include <boost/units/io.hpp>

namespace dStorm {
namespace image {

template <int Dim>
void MetaInfo<Dim>::set_resolution( int dimension, const traits::ImageResolution& resolution )
{
    resolutions_[dimension] = resolution;
}

template class MetaInfo<1>;
template class MetaInfo<2>;
template class MetaInfo<3>;

}
}
