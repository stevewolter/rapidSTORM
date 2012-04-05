#include <boost/units/Eigen/Core>
#include "depth_range.h"
#include "equifocal_plane.h"
#include <dStorm/traits/optics.h>
#include <boost/variant/apply_visitor.hpp>
#include "Spline.h"
#include <stdexcept>

namespace dStorm {
namespace traits {

using namespace boost::units;

struct depth_range_visitor
: public boost::static_visitor< boost::optional<ZRange> >
{
public:
    boost::optional<ZRange> operator()( const traits::No3D& ) const
        { return boost::optional<ZRange>(); }
    boost::optional<ZRange> operator()( const traits::Polynomial3D& p ) const { 
        if ( ! p.focal_planes() || ! p.z_range() )
            throw std::logic_error("Polynomial3D is incomplete in get_z_range");
        return ZRange( 
            samplepos::Scalar((*p.focal_planes() - *p.z_range()).minCoeff()),
            samplepos::Scalar((*p.focal_planes() + *p.z_range()).maxCoeff()) );
    }
    boost::optional<ZRange> operator()( const traits::Spline3D& s ) const { 
        return ZRange( s.get_spline()->lowest_z(), s.get_spline()->highest_z() );
    }
};

boost::optional<ZRange> get_z_range( const DepthInfo& o ) {
    return boost::apply_visitor( depth_range_visitor(), o );
}

boost::optional<ZRange> merge_z_range( const boost::optional<ZRange>& s1, const boost::optional<ZRange>& s2 )
{
    if ( !s1 )
        return s2;
    else if ( ! s2 )
        return s1;
    else {
        return hull(*s1, *s2);
    }
}

}
}
