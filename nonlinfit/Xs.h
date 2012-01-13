#ifndef NONLINFIT_XS_H
#define NONLINFIT_XS_H

#include <iosfwd>

namespace nonlinfit {

/** Generic input parameter. Xs<0>, Xs<1>, Xs<2> ... are equivalent to math's
 *  usage of x, y and z. */
template <int Number, typename LengthUnit>
struct Xs {
    typedef LengthUnit Unit;
    static const int Dimension = Number, value = Number;
};

template <int Dim, typename Length>
inline std::ostream& operator<<(std::ostream& o, Xs<Dim,Length>);

}

#endif

