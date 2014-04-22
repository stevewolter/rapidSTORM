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
    typedef nonlinfit::DataChunk<typename CoreData::Number, ChunkSize> DataChunk;
    Data data;
    std::vector<DataChunk> data_chunks;
    int current_point;

    DataFacade();

    typedef typename CoreData::data_point value_type;
    typedef const value_type& const_reference;

    void clear() { data.clear(); data_chunks.clear(); }
    void reserve(int n) { data.reserve(n/ChunkSize+1); data_chunks.reserve(n/ChunkSize+1); }
    void push_back( const value_type& );
    void pad_last_chunk();

    struct const_iterator;
    const_iterator begin() const;
    const_iterator end() const;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
}

#endif
