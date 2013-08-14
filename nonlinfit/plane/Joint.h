#ifndef NONLINFIT_PLANE_JOINT_H
#define NONLINFIT_PLANE_JOINT_H

#include "fwd.h"
#include <nonlinfit/Xs.h>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>

namespace nonlinfit {
namespace plane {

/** ComputationWay for joint (non-disjoint) computing on a plane.
 *  Joint computation is always possible and makes no assumptions about
 *  the layout of data points. However, it is also slower and less accurate
 *  than disjoint computation. The two parameters for joint computation
 *  are treated equally.
 *
 *  \tparam _ChunkSize The number of data points to be computed parallely.
 *                     Parallel computation allows the Eigen library to use
 *                     SSE and other processor features, but also forces the
 *                     data to be padded to appropriate size.
 *  \sa Disjoint
 **/
template <typename Num, int _ChunkSize, typename FirstParam, typename SecondParam>
struct Joint {
    static const int ChunkSize = _ChunkSize;
    typedef JointData<Num,_ChunkSize> Data;
    typedef Num Number;
};

/** Boost.MPL metafunction creating a Joint instance with Xs variables. */
template <typename Num, int _ChunkSize>
struct xs_joint {
    typedef Joint<Num,_ChunkSize, Xs<0>, Xs<1> > type;
};

}
}

#endif
