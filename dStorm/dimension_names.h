#ifndef DSTORM_DIMENSION_NAMES_H
#define DSTORM_DIMENSION_NAMES_H

#include <string>

namespace dStorm {

template <int Dimension>
std::string spatial_dimension_name();

template <>
inline std::string spatial_dimension_name<0>() { return "x"; }
template <>
inline std::string spatial_dimension_name<1>() { return "y"; }
template <>
inline std::string spatial_dimension_name<2>() { return "z"; }

}

#endif
