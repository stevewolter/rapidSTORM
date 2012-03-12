#ifndef DSTORM_TRAITS_POLYNOMIAL3D_H
#define DSTORM_TRAITS_POLYNOMIAL3D_H

namespace dStorm {
namespace polynomial_3d {

typedef int Term;
static const Term FirstTerm = 1, Order = 4, LastTerm = Order, Quadratic = 2;

inline int offset( Term t ) { return t - FirstTerm; }

}
}

#endif
