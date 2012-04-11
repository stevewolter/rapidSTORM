#include <boost/units/Eigen/Core>
#include "depth_range.h"
#include "equifocal_plane.h"
#include <boost/variant/apply_visitor.hpp>
#include "Spline3D.h"
#include <stdexcept>

namespace dStorm {
namespace threed_info {

using namespace boost::units;

struct depth_range_visitor
: public boost::static_visitor< ZRange >
{
public:
    ZRange operator()( const No3D& ) const
        { return ZRange(); }
    ZRange operator()( const Polynomial3D& p ) const { 
        if ( ! p.focal_planes() || ! p.z_range() )
            throw std::logic_error("Polynomial3D is incomplete in get_z_range");
        ZRange rv;
        rv += ZInterval(
            samplepos::Scalar((*p.focal_planes() - *p.z_range()).minCoeff()),
            samplepos::Scalar((*p.focal_planes() + *p.z_range()).maxCoeff()) );
        return rv;
    }
    ZRange operator()( const Spline3D& s ) const { 
        return s.z_range();
    }
};

ZRange get_z_range( const DepthInfo& o ) {
    return boost::apply_visitor( depth_range_visitor(), o );
}

}
}
