#include <boost/units/Eigen/Core>
#include "equifocal_plane.h"
#include <boost/variant/apply_visitor.hpp>

namespace dStorm {
namespace threed_info {

using namespace boost::units;

struct equifocal_plane_visitor
: public boost::static_visitor< quantity<si::length,float> >
{
public:
    quantity<si::length,float> operator()( const No3D& ) const
        { return 0 * si::meter; }
    quantity<si::length,float> operator()( const Polynomial3D& p ) const
        { return quantity<si::length,float>(p.focal_planes()->x() + p.focal_planes()->y()) / 2.0f; }
    quantity<si::length,float> operator()( const Spline3D& s ) const { 
        return quantity<si::length,float>(s.equifocal_plane());
    }
};

quantity<si::length,float> equifocal_plane( const DepthInfo& o ) {
    return boost::apply_visitor( equifocal_plane_visitor(), o );
}

}
}
