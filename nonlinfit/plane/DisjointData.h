#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_DISJOINT_DATA_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_DISJOINT_DATA_H

#include <Eigen/Core>
#include <Eigen/StdVector>
#include <vector>
#include <boost/static_assert.hpp>

#include "nonlinfit/plane/GenericData.h"

namespace nonlinfit {
namespace plane {

/** Data structure for the Disjoint computation way. 
 *  Disjoint data store the first and second dimension
 *  independently, with the outer dimension residing in
 *  the #xs member and the inner dimension residing in
 *  DataRow::inputs. */
template <typename Number_, int _ChunkSize>
struct DisjointData : public GenericData
{
    BOOST_STATIC_ASSERT((_ChunkSize != Eigen::Dynamic));
    static const int ChunkSize = _ChunkSize;
    typedef Number_ Number;
    typedef Eigen::Array<Number,1,1> Input;

    struct DataRow {
        typedef Eigen::Array<Number, ChunkSize, 1> Output;

        /** The Y coordinate for the current row in LengthUnit. */
        Input inputs;
        /** The measurements for the matching row in #inputs. */
        Output output;
        /** The value of \f$x \ln x\f$ for each \f$ x \f$ in #output. */
        Output logoutput;
        /** The background estimate for each pixel. */
        Output background;
        /** The difference between the measurements and a function's last
          *  evaluation on these data. */
        mutable Output residues;

        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };

    typedef std::vector<DataRow, Eigen::aligned_allocator<DataRow> > Data;

    Data data;
    /** The X coordinates for all DataRow instances in the data vector. */
    Eigen::Array<Number, _ChunkSize, 1> xs;

    Eigen::Matrix<Number, 2, 1> get_coordinate(const DataRow& chunk, int index) const {
        Eigen::Matrix<Number, 2, 1> result;
        result.x() = xs[index];
        result.y() = chunk.inputs(0,0);
        return result;
    }
};

}
}

#endif
