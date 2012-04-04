#ifndef NONLINFIT_IMAGE_PLANE_DATAPOINT_H
#define NONLINFIT_IMAGE_PLANE_DATAPOINT_H

#include <Eigen/Core>
#include <boost/units/quantity.hpp>
#include <boost/units/Eigen/Array>

namespace nonlinfit {
namespace plane {

template <typename LengthUnit, typename Number>
struct DataPoint {
public:
    typedef boost::units::quantity<LengthUnit,Number> Length;
    typedef Number Intensity;

    DataPoint() : o(0), logo(0), res(0) {}
    DataPoint( Length x, Length y, Number output, Number logoutput, Number residues )
        : o(output), logo(logoutput), res(residues) { l_[0] = x; l_[1] = y; }
    template <typename Derived>
    DataPoint( const Eigen::DenseBase<Derived>& p, Number v ) {
        o = v;
        l_[0] = boost::units::quantity< LengthUnit >(p.x());
        l_[1] = boost::units::quantity< LengthUnit >(p.y());
        logo = (o < 1E-10) ? -23*o : o * log(o);
        res = 0;
    }

    Length x() const { return l_[0]; }
    Length y() const { return l_[1]; }
    Length position( int dim ) const { return l_[dim]; }
    Eigen::Matrix< Length, 2, 1 > position() const { 
        Eigen::Matrix< Length, 2, 1 > rv;
        for (int i = 0; i < 2; ++i) rv[i] = l_[i];
        return rv;
    }
    Number value() const { return o; }
    Number logoutput() const { return logo; }
    Number residue() const { return res; }
private:
    Length l_[2];
    Number o, logo, res;
};

}
}

#endif
