#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_DISJOINT_DATA_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_DISJOINT_DATA_H

#include <Eigen/Core>
#include <Eigen/StdVector>
#include <vector>
#include "GenericData.h"
#include "fwd.h"
#include <boost/static_assert.hpp>

namespace nonlinfit {
namespace plane {

/** Data structure for the Disjoint computation way. 
 *  Disjoint data store the first and second dimension
 *  independently, with the outer dimension residing in
 *  the #xs member and the inner dimension residing in
 *  DataRow::inputs. */
template <typename Number, typename LengthUnit, int _ChunkSize>
struct DisjointData
    : public GenericData<LengthUnit>
{
    BOOST_STATIC_ASSERT((_ChunkSize != Eigen::Dynamic));
    typedef Eigen::Array<Number,1,1> Input;
    typedef Eigen::Array<Number, _ChunkSize, 1> Output;

    struct DataRow {
        typedef DisjointData::Output Output;
        /** The Y coordinate for the current row in LengthUnit. */
        Input inputs;
        /** The measurements for the matching row in #xs and the row's Y. */
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
    /** The X coordinates for all DataRow instances in the data vector. */
    Eigen::Array<Number, _ChunkSize, 1> xs;

    typedef DataRow value_type;
    typedef typename Data::const_iterator const_iterator;
    const_iterator begin() const { return data.begin(); }
    const_iterator end() const { return data.end(); }
    void clear() { data.clear(); }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
}

#endif
