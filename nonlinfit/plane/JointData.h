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

    struct DataRow {
        /** The X and Y coordinates for the current row. */
        Input inputs;
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };

    double get_x( const DataRow& chunk, int in_chunk ) const { return chunk.inputs(in_chunk, 0); }
    double get_y( const DataRow& chunk, int in_chunk ) const { return chunk.inputs(in_chunk, 1); }
    void set( DataRow& chunk, int in_chunk, const data_point& );
};

template <typename Number, int ChunkSize>
class JointData
: public DataFacade< JointCoreData<Number,ChunkSize> >,
  public GenericData
{
public:
    JointData() {}
    template <typename ONum, int Width>
    JointData( const DisjointData< ONum, Width >& );
    template <typename ONum, int Width>
    JointData( const JointData< ONum, Width >& );
};

}
}

#endif
