#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_DISJOINT_DATA_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_DISJOINT_DATA_H

#include <Eigen/Core>
#include <Eigen/StdVector>
#include <vector>
#include "nonlinfit/plane/GenericData.h"
#include "nonlinfit/plane/fwd.h"
#include <boost/static_assert.hpp>
#include "nonlinfit/plane/DataFacade.h"
#include "nonlinfit/DataChunk.h"

namespace nonlinfit {
namespace plane {

/** Data structure for the Disjoint computation way. 
 *  Disjoint data store the first and second dimension
 *  independently, with the outer dimension residing in
 *  the #xs member and the inner dimension residing in
 *  DataRow::inputs. */
template <typename Number_, int _ChunkSize>
struct DisjointCoreData
{
    BOOST_STATIC_ASSERT((_ChunkSize != Eigen::Dynamic));
    static const int ChunkSize = _ChunkSize;
    typedef Number_ Number;
    typedef Eigen::Array<Number,1,1> Input;
    typedef DataPoint<Number> data_point;

    struct DataRow {
        /** The Y coordinate for the current row in LengthUnit. */
        Input inputs;
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };
    /** The X coordinates for all DataRow instances in the data vector. */
    Eigen::Array<Number, _ChunkSize, 1> xs;

    double get_x( const DataRow& chunk, int in_chunk ) const { return xs[in_chunk]; }
    double get_y( const DataRow& chunk, int in_chunk ) const { return chunk.inputs(0,0); }
    void set( DataRow& chunk, int in_chunk, const data_point& );
};

template <typename Number, int ChunkSize>
struct DisjointData
: public DataFacade< DisjointCoreData<Number,ChunkSize> >,
  public GenericData
{
};

}
}

#endif
