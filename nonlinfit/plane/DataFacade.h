#ifndef NONLINFIT_PLANE_DATAFACADE_H
#define NONLINFIT_PLANE_DATAFACADE_H

#include <Eigen/StdVector>
#include <vector>
#include "GenericData.h"
#include "fwd.h"
#include "DataPoint.h"

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
    int current_point;

    DataFacade();

    struct ChunkView;
    inline ChunkView chunk_view() const ;

    typedef typename CoreData::data_point value_type;
    typedef const value_type& const_reference;

    void clear() { data.clear(); }
    void reserve(int n) { data.reserve(n/ChunkSize+1); }
    void push_back( const value_type& );
    void pad_last_chunk();

    struct const_iterator;
    const_iterator begin() const;
    const_iterator end() const;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

template <typename CoreData>
struct DataFacade<CoreData>::ChunkView
{
    friend class DataFacade;
    const DataFacade& d;
    ChunkView( const DataFacade& d ) : d(d) {}
public:
    typedef DataRow value_type;
    typedef typename Data::const_iterator const_iterator;
    const_iterator begin() const { return d.data.begin(); }
    const_iterator end() const { return d.data.end(); }
};

template <typename CoreData>
typename DataFacade<CoreData>::ChunkView
DataFacade<CoreData>::chunk_view() const
    { return ChunkView(*this); }

}
}

#endif
