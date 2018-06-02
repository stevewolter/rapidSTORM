#ifndef NONLINFIT_XS_H
#define NONLINFIT_XS_H

#include <iosfwd>

#include <boost/mpl/int.hpp>

namespace nonlinfit {

/** Generic input parameter. Xs<0>, Xs<1>, Xs<2> ... are equivalent to math's
 *  usage of x, y and z. */
template <int Number>
struct Xs : public boost::mpl::int_<Number> {
    static const int Dimension = Number;
};

template <int Dim>
inline std::ostream& operator<<(std::ostream& o, Xs<Dim>);

}

#endif

