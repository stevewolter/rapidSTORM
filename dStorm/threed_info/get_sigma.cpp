#include "get_sigma.h"
#include <boost/variant/apply_visitor.hpp>

namespace dStorm {
namespace threed_info {

struct get_sigma_visitor 
: public boost::static_visitor< Sigma >
{
    const Direction dir;
    const ZPosition z;
    get_sigma_visitor( Direction dir, ZPosition z ) : dir(dir), z(z) {}
    Sigma operator()( const No3D& p ) const { return p.sigma[dir]; }
    Sigma operator()( const Polynomial3D& p ) const { return p.get_sigma( dir, z ); }
    Sigma operator()( const Spline3D& p ) const { return p.get_sigma( dir, z ); } 
};

Sigma get_sigma( const DepthInfo& o, Direction dir, ZPosition z ) {
    return boost::apply_visitor( get_sigma_visitor(dir,z), o );
}

}
}
