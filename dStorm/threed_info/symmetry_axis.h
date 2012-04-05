#ifndef DSTORM_THREED_INFO_IS_SYMMETRIC_IN_Z_H
#define DSTORM_THREED_INFO_IS_SYMMETRIC_IN_Z_H

#include "DepthInfo.h"

namespace dStorm {
namespace threed_info {

struct NoDepthInformation {};
struct Unsymmetric {};
struct AxisSymmetric { boost::units::quantity<boost::units::si::length> axis; };

typedef boost::variant< NoDepthInformation, Unsymmetric, AxisSymmetric > Symmetry;
Symmetry symmetry_axis( const DepthInfo& );
Symmetry merge_symmetries( const Symmetry&, const Symmetry& );
bool has_z_information( const Symmetry& );

}
}

#endif
