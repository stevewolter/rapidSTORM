#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_JOINT_DATA_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_JOINT_DATA_H

#include <Eigen/Core>
#include <Eigen/StdVector>
#include "nonlinfit/plane/GenericData.h"
#include "nonlinfit/plane/fwd.h"
#include <vector>
#include <boost/static_assert.hpp>
#include "nonlinfit/plane/DataPoint.h"
#include "nonlinfit/plane/DataFacade.h"
#include "nonlinfit/DataChunk.h"

namespace nonlinfit {
namespace plane {

/** Data structure for the Joint computation way. */
template <typename Number_, int ChunkSize_>
class JointCoreData
{
    BOOST_STATIC_ASSERT((ChunkSize_ != Eigen::Dynamic));
  public:
    static const int ChunkSize = ChunkSize_;
    typedef Number_ Number;
    typedef DataPoint<Number> data_point;
    typedef Eigen::Array<Number, ChunkSize, 2> Input;

    struct DataRow : public nonlinfit::DataChunk<Number_,ChunkSize_> {
        /** The X and Y coordinates for the current row. */
        Input inputs;
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };
};

template <typename Number, int ChunkSize>
class JointData
: public DataFacade< JointCoreData<Number,ChunkSize> >,
  public GenericData {};

}
}

#endif
