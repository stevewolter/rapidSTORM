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

    DataPoint() : o(0) {}
    DataPoint( Length x, Length y, Number output )
        : o(output) { l_[0] = x; l_[1] = y; }
    template <typename Derived>
    DataPoint( const Eigen::DenseBase<Derived>& p, Number v ) {
        o = v;
        l_[0] = boost::units::quantity< LengthUnit >(p.x());
        l_[1] = boost::units::quantity< LengthUnit >(p.y());
    }

    Length x() const { return l_[0]; }
    Length y() const { return l_[1]; }
    Length position( int dim ) const { return l_[dim]; }
    Number value() const { return o; }
private:
    Length l_[2];
    Number o;
};

}
}

#endif
