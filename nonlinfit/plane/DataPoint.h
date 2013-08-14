#ifndef NONLINFIT_IMAGE_PLANE_DATAPOINT_H
#define NONLINFIT_IMAGE_PLANE_DATAPOINT_H

#include <Eigen/Core>
#include <boost/units/quantity.hpp>
#include <boost/units/Eigen/Array>

namespace nonlinfit {
namespace plane {

template <typename Number>
struct DataPoint {
public:
    typedef Number Intensity;

    DataPoint() : o(0), logo(0), res(0) {}
    DataPoint( Number x, Number y, Number output, Number logoutput, Number residues )
        : o(output), logo(logoutput), res(residues) { l_[0] = x; l_[1] = y; }
    template <typename Derived>
    DataPoint( const Eigen::DenseBase<Derived>& p, Number v ) {
        o = v;
        l_[0] = p.x();
        l_[1] = p.y();
        logo = (o < 1E-10) ? -23*o : o * log(o);
        res = 0;
    }

    Number x() const { return l_[0]; }
    Number y() const { return l_[1]; }
    Number position( int dim ) const { return l_[dim]; }
    Eigen::Matrix< Number, 2, 1 > position() const { 
        Eigen::Matrix< Number, 2, 1 > rv;
        for (int i = 0; i < 2; ++i) rv[i] = l_[i];
        return rv;
    }
    Number value() const { return o; }
    Number logoutput() const { return logo; }
    Number residue() const { return res; }
private:
    Number l_[2];
    Number o, logo, res;
};

}
}

#endif
