#ifndef NONLINFIT_PLANE_FWD_H
#define NONLINFIT_PLANE_FWD_H

namespace nonlinfit {

/** Contains tags and evaluation functions for two-dimensional datasets. */
namespace plane {

class GenericData;
template <typename Num, int ChunkSize> class DisjointData;
template <typename Num, int ChunkSize> class JointData;

template <typename Num, int ChunkSize, typename P1, typename P2> class Disjoint;
template <typename Num, int ChunkSize, typename P1, typename P2> class Joint;

template <typename _Tag, typename _Metric>
struct Distance;

/** Metric defined by the sum of squared differences to a data set.
 *  The distance \f$g\f$ for a squared deviations metric is defined by the
 *  set of data positions \f$X\f$ in the data and the base function \f$f\f$ as 
 *  \f$g = \sum_{x \in X} \left(f(x) - data(x)\right)^2\f$. */
struct squared_deviations {};
/** Function adapter computing the negative likelihood of a Poisson process 
 *  with expected value equal to the base function. */
struct negative_poisson_likelihood {};

}
}

#endif
