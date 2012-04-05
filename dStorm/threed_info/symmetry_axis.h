#ifndef DSTORM_THREED_INFO_IS_SYMMETRIC_IN_Z_H
#define DSTORM_THREED_INFO_IS_SYMMETRIC_IN_Z_H

#include <dStorm/traits/DepthInfo.h>

namespace dStorm {
namespace traits {

struct Unsymmetric {};
struct TotallySymmetric {};
struct AxisSymmetric { boost::units::quantity<boost::units::si::length> axis; };

typedef boost::variant< Unsymmetric, TotallySymmetric, AxisSymmetric > Symmetry;
Symmetry symmetry_axis( const DepthInfo& );
Symmetry merge_symmetries( const Symmetry&, const Symmetry& );

}
}

#endif
