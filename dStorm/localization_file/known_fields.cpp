#include "known_fields.h"
#include "fields_impl.h"

namespace dStorm {
namespace LocalizationFile {
namespace field {
namespace properties {

template <>
const std::string Spatial<0>::semantic = "x-position";
template <>
const std::string Spatial<1>::semantic = "y-position";
template <>
const std::string Spatial<2>::semantic = "z-position";

const std::string Time::semantic = "frame number";
const std::string Amplitude::semantic = "emission strength";
const std::string TwoKernelImprovement::semantic = "two kernel improvement";

}

using namespace properties;

template class KnownWithResolution< Spatial<0> >;
template class KnownWithResolution< Spatial<1> >;
template class KnownWithResolution< Spatial<2> >;
template class KnownWithResolution< properties::Time >;
template class Known< properties::Amplitude >;
template class Known< properties::TwoKernelImprovement >;

}
}
}
