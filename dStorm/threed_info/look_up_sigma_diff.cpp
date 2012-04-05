#include <boost/units/Eigen/Core>
#include "look_up_sigma_diff.h"
#include "equifocal_plane.h"
#include <dStorm/traits/optics.h>
#include <boost/variant/apply_visitor.hpp>
#include <dStorm/threed_info/Spline.h>
#include <boost/units/cmath.hpp>
#include <boost/units/io.hpp>
#include <boost/lexical_cast.hpp>

namespace dStorm {
namespace traits {

using namespace boost::units;

class look_up_sigma_diff_visitor
: public boost::static_visitor< quantity<si::length,float> >
{
    quantity<si::length> sigma_diff;
public:
    look_up_sigma_diff_visitor( quantity<si::length> sigma_diff ) 
        : sigma_diff(sigma_diff) {}
    quantity<si::length,float> operator()( const traits::No3D& ) const
        { return 0 * si::meter; }
    quantity<si::length,float> operator()( const traits::Polynomial3D& p ) const {
        quantity<si::length> lower_bound = p.lowest_z(), upper_bound = p.highest_z(),
                             precision = 1e-9 * si::meter;

        /* Switch bounds if the gradient is negative, so we can assume in 
        * the rest of the search that lower_bound is at the Y-larger-X end
        * (not necessarily the low-Z end). */
        boost::optional< Polynomial3D::Sigma > lower_sigma, upper_sigma, test_sigma;
        lower_sigma = p.get_sigma_diff( lower_bound );
        upper_sigma = p.get_sigma_diff( upper_bound );
        if ( ! lower_sigma )
            throw std::runtime_error("PSF width cannot be evaluated at Z " + boost::lexical_cast<std::string>(lower_bound) );
        if ( ! upper_sigma )
            throw std::runtime_error("PSF width cannot be evaluated at Z " + boost::lexical_cast<std::string>(upper_bound) );
        if ( *lower_sigma > *upper_sigma )
            std::swap( lower_bound, upper_bound );

        if ( *lower_sigma > sigma_diff || *upper_sigma < sigma_diff )
            return equifocal_plane(p);
        else {
            while ( abs( upper_bound - lower_bound ) > precision ) {
                quantity<si::length> test_x = (lower_bound + upper_bound) / 2.0;
                test_sigma = *p.get_sigma_diff( test_x );
                if ( ! test_sigma )
                    throw std::runtime_error("PSF width cannot be evaluated at Z " + boost::lexical_cast<std::string>(test_x) );
                else if ( *test_sigma > sigma_diff )
                    upper_bound = test_x;
                else
                    lower_bound = test_x;
            }

            return quantity<si::length,float>(upper_bound + lower_bound) / 2.0f;
        }
    }
    quantity<si::length,float> operator()( const traits::Spline3D& s ) const
    { 
        return quantity<si::length,float>(s.get_spline()->look_up_sigma_diff( sigma_diff, 1E-8 * si::meter )
            .get_value_or( s.equifocal_plane() ));
    }
};

quantity<si::length,float> look_up_sigma_diff( const DepthInfo& o, quantity<si::length> sigma_diff ) {
    return boost::apply_visitor( look_up_sigma_diff_visitor(sigma_diff), o );
}

}
}
