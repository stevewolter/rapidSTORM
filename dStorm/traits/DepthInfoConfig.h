#ifndef DSTORM_TRAITS_DEPTHINFOCONFIG_H
#define DSTORM_TRAITS_DEPTHINFOCONFIG_H

#include "DepthInfo.h"
#include <simparm/Node.hh>

namespace dStorm {
namespace traits {

class PlaneConfig;

struct ThreeDConfig {
    virtual ~ThreeDConfig() {}
    virtual DepthInfo make_traits( const PlaneConfig& ) const = 0;
    virtual void read_traits( const DepthInfo&, PlaneConfig& ) = 0;
    virtual void set_context( PlaneConfig& ) = 0;
    virtual simparm::Node& getNode() = 0;
    operator simparm::Node&() { return getNode(); }
    operator const simparm::Node&() const { return const_cast<ThreeDConfig&>(*this).getNode(); }
    virtual ThreeDConfig* clone() const = 0;
};

std::auto_ptr< ThreeDConfig > make_no_3d_config();
std::auto_ptr< ThreeDConfig > make_polynomial_3d_config();
std::auto_ptr< ThreeDConfig > make_spline_3d_config();

}
}

#endif
