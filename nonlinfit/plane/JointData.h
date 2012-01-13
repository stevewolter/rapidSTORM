#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_JOINT_DATA_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_JOINT_DATA_H

#include <Eigen/Core>
#include <Eigen/StdVector>
#include "GenericData.h"
#include "fwd.h"
#include <vector>
#include <boost/static_assert.hpp>

namespace nonlinfit {
namespace plane {

/** Data structure for the Joint computation way. */
template <typename Number, typename LengthUnit, int ChunkSize>
class JointData
: public GenericData<LengthUnit>
{
    BOOST_STATIC_ASSERT((ChunkSize != Eigen::Dynamic));
  public:
    typedef Eigen::Array<Number, ChunkSize, 2> Input;
    typedef Eigen::Array<Number, ChunkSize, 1> Output;

    struct DataRow {
        typedef JointData::Output Output;
        /** The X and Y coordinates for the current row in LengthUnit. */
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
  private:
    typedef std::vector<DataRow, Eigen::aligned_allocator<DataRow> > Data;
    Data data;

  public:
    typedef DataRow value_type;
    typedef typename Data::const_iterator const_iterator;
    const_iterator begin() const { return data.begin(); }
    const_iterator end() const { return data.end(); }
    void clear() { data.clear(); }
    void reserve(int n) { data.reserve(n); }

    struct data_point_iterator;
    data_point_iterator point_back_inserter();

    struct DataPoint {
        friend class DataPointReference;
        Number x, y, o;
      public:
        DataPoint( Number x, Number y, Number output )
            : x(x), y(y), o(output) {}
        template <typename Derived>
        DataPoint( const Eigen::DenseBase<Derived>& p, Number v ) {
            o = v;
            x = boost::units::quantity< LengthUnit >(p.x()).value();
            y = boost::units::quantity< LengthUnit >(p.y()).value();
        }
    };

  private:
    struct DataPointReference;

  public:
    JointData() {}
    template <typename ONum, int Width>
    JointData( const DisjointData< ONum, LengthUnit,Width >& );
    template <typename ONum, int Width>
    JointData( const JointData< ONum, LengthUnit,Width >& );
};

}
}

#endif
