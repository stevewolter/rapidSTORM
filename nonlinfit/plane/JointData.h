#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_JOINT_DATA_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_JOINT_DATA_H

#include <Eigen/Core>
#include <Eigen/StdVector>
#include <vector>
#include <boost/static_assert.hpp>

#include "nonlinfit/plane/GenericData.h"

namespace nonlinfit {
namespace plane {

/** Data structure for the Joint computation way. */
template <typename Number_, int ChunkSize_>
class JointData : public GenericData
{
    BOOST_STATIC_ASSERT((ChunkSize_ != Eigen::Dynamic));
  public:
    static const int ChunkSize = ChunkSize_;
    typedef Number_ Number;
    typedef Eigen::Array<Number, ChunkSize, 2> Input;

    struct DataRow {
        typedef Eigen::Array<Number, ChunkSize, 1> Output;

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

    typedef std::vector<DataRow, Eigen::aligned_allocator<DataRow> > Data;
    Data data;
};

}
}

#endif
