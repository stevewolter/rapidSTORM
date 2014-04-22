#ifndef NONLINFIT_PLANE_DATAFACADE_H
#define NONLINFIT_PLANE_DATAFACADE_H

#include <Eigen/StdVector>
#include <vector>
#include "nonlinfit/plane/GenericData.h"
#include "nonlinfit/plane/fwd.h"
#include "nonlinfit/plane/DataPoint.h"
#include "nonlinfit/DataChunk.h"

namespace nonlinfit {
namespace plane {

template <typename CoreData>
struct DataFacade
: public CoreData
{
    static const int ChunkSize = CoreData::ChunkSize;
    typedef typename CoreData::DataRow DataRow;
    typedef std::vector<DataRow, Eigen::aligned_allocator<DataRow> > Data;
    Data data;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
}

#endif
