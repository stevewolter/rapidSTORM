#include "symmetry_axis.h"
#include <boost/variant/apply_visitor.hpp>

namespace dStorm {
namespace threed_info {

class merge_symmetries_visitor
: public boost::static_visitor<Symmetry>
{
public:
    template <typename Second>
    Symmetry operator()( Unsymmetric, Second ) const 
        { return Unsymmetric(); }
    Symmetry operator()( AxisSymmetric, Unsymmetric s ) const 
        { return s; }
    Symmetry operator()( AxisSymmetric s1, AxisSymmetric s2 ) const { 
        if ( (s1.axis - s2.axis) < 1E-9 * boost::units::si::meter ) 
            return s1;
        else
            return Unsymmetric();
    }
    Symmetry operator()( AxisSymmetric s, NoDepthInformation ) const
        { return s; }
    template <typename Second>
    Symmetry operator()( NoDepthInformation, Second s ) const 
        { return s; }

};

Symmetry merge_symmetries( const Symmetry& s1, const Symmetry& s2 ) {
    return boost::apply_visitor( merge_symmetries_visitor(), s1, s2 );
}

struct symmetry_visitor
: public boost::static_visitor<Symmetry>
{
    Symmetry operator()( const No3D& ) const { return NoDepthInformation(); }
    Symmetry operator()( const Polynomial3D& p ) const {
        if ( ( p.focal_planes()->x() - p.focal_planes()->y() ) < 1E-9 * boost::units::si::meter )
        {
            AxisSymmetric rv;
            rv.axis = p.focal_planes()->x();
            return rv;
        } else
            return Unsymmetric();
    }
    Symmetry operator()( const Spline3D& ) const { return Unsymmetric(); }
};

Symmetry symmetry_axis( const DepthInfo& d ) {
    return boost::apply_visitor( symmetry_visitor(), d );
}

struct has_z_information_visitor : public boost::static_visitor<bool> {
    bool operator()( NoDepthInformation ) const { return false; }
    bool operator()( Unsymmetric ) const { return true; }
    bool operator()( AxisSymmetric ) const { return true; }
};

bool has_z_information( const Symmetry& d ) {
    return boost::apply_visitor( has_z_information_visitor(), d );
}

}
}
