#ifndef DSTORM_3DFITTER_CONSTANTTYPES_H
#define DSTORM_3DFITTER_CONSTANTTYPES_H

#include "Exponential3D.hh"
#include <dStorm/units/nanoresolution.h>

namespace fitpp {
namespace Exponential3D {

template <int Widening>
struct ConstantTypes;

template <>
struct ConstantTypes< Holtzer > {
    typedef dStorm::nanoresolution ResolutionUnit;
};

template <>
struct ConstantTypes< Zhuang > {
    typedef boost::units::divide_typeof_helper<
                    dStorm::nanoresolution,
                    boost::units::si::nanolength
            >::type ResolutionUnit;
};

}
}

namespace boost {
namespace units {

std::string name_string(const fitpp::Exponential3D::ConstantTypes<fitpp::Exponential3D::Zhuang>::ResolutionUnit&);
std::string symbol_string(const fitpp::Exponential3D::ConstantTypes<fitpp::Exponential3D::Holtzer>::ResolutionUnit&);

}
}

#endif
