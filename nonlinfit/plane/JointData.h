#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_JOINT_DATA_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_JOINT_DATA_H

#include <Eigen/Core>
#include <Eigen/StdVector>
#include "GenericData.h"
#include "fwd.h"
#include <vector>
#include <boost/static_assert.hpp>
#include "DataPoint.h"
#include "DataFacade.h"

namespace nonlinfit {
namespace plane {

/** Data structure for the Joint computation way. */
template <typename Number, int ChunkSize_>
class JointCoreData
{
    BOOST_STATIC_ASSERT((ChunkSize_ != Eigen::Dynamic));
  public:
    static const int ChunkSize = ChunkSize_;
    typedef DataPoint<Number> data_point;
    typedef Eigen::Array<Number, ChunkSize, 2> Input;
    typedef Eigen::Array<Number, ChunkSize, 1> Output;

    struct DataRow {
        typedef JointCoreData::Output Output;
        /** The X and Y coordinates for the current row. */
        Input inputs;
        /** The measurements for the matching row in #inputs. */
        Output output;
        /** The value of \f$x \ln x\f$ for each \f$ x \f$ in #output. */
        Output logoutput;
        /** The difference between the measurements and a function's last
         *  evaluation on these data. */
        mutable Output residues;
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };

    data_point get( const DataRow& chunk, int in_chunk ) const;
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
