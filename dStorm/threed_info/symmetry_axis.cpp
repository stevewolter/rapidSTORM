#include "symmetry_axis.h"
#include <boost/variant/apply_visitor.hpp>

namespace dStorm {
namespace traits {

class merge_symmetries_visitor
: public boost::static_visitor<traits::Symmetry>
{
public:
    template <typename Second>
    traits::Symmetry operator()( traits::Unsymmetric, Second ) const 
        { return traits::Unsymmetric(); }
    traits::Symmetry operator()( traits::AxisSymmetric, traits::Unsymmetric s ) const 
        { return s; }
    traits::Symmetry operator()( traits::AxisSymmetric s1, traits::AxisSymmetric s2 ) const { 
        if ( (s1.axis - s2.axis) < 1E-9 * boost::units::si::meter ) 
            return s1;
        else
            return traits::Unsymmetric();
    }
    traits::Symmetry operator()( traits::AxisSymmetric s, traits::TotallySymmetric ) const
        { return s; }
    template <typename Second>
    traits::Symmetry operator()( traits::TotallySymmetric, Second s ) const 
        { return s; }

};

Symmetry merge_symmetries( const Symmetry& s1, const Symmetry& s2 ) {
    return boost::apply_visitor( merge_symmetries_visitor(), s1, s2 );
}

struct symmetry_visitor
: public boost::static_visitor<traits::Symmetry>
{
    traits::Symmetry operator()( const traits::No3D& ) const { return traits::TotallySymmetric(); }
    traits::Symmetry operator()( const traits::Polynomial3D& p ) const {
        if ( ( p.focal_planes()->x() - p.focal_planes()->y() ) < 1E-9 * boost::units::si::meter )
        {
            traits::AxisSymmetric rv;
            rv.axis = p.focal_planes()->x();
            return rv;
        } else
            return traits::Unsymmetric();
    }
    traits::Symmetry operator()( const traits::Spline3D& ) const { return traits::Unsymmetric(); }
};

Symmetry symmetry_axis( const DepthInfo& d ) {
    return boost::apply_visitor( symmetry_visitor(), d );
}

}
}
