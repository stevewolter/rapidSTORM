#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_JOINT_DATA_IMPL_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_JOINT_DATA_IMPL_H

#include "JointData.h"
#include <boost/units/Eigen/Array>
#include <boost/iterator/iterator_facade.hpp>

namespace nonlinfit {
namespace plane {

template <typename Number, typename LengthUnit, int ChunkSize>
struct JointData<Number, LengthUnit,ChunkSize>::DataPointReference {
    friend class data_point_iterator;

    DataRow *d; 
    int i;
    DataPointReference() {}
    DataPointReference( DataRow *d, int i ) : d(d), i(i) {}

  public:
    DataPointReference& operator=( const DataPoint& p ) {
        d->inputs( i, 0 ) = p.x;
        d->inputs( i, 1 ) = p.y;
        d->output[i] = p.o;
        d->logoutput[i] = (p.o < 1E-10) ? -23*p.o : p.o * log(p.o) ;
        return *this;
    }
};

template <typename Num, typename LengthUnit, int ChunkSize>
struct JointData<Num,LengthUnit,ChunkSize>::data_point_iterator
: public boost::iterator_facade<
    data_point_iterator,
    DataPointReference,
    std::output_iterator_tag >
{
    data_point_iterator( JointData& d ) : d(d), index(ChunkSize) {}

    void pad_last_chunk() {
        if ( index < ChunkSize/2 )
            d.data.pop_back();
        else while ( index < ChunkSize ) {
            DataRow& b = d.data.back();
            b.inputs.row(index) = b.inputs.row( index - ChunkSize/2 );
            b.output.row(index) = b.output.row( index - ChunkSize/2 );
            b.logoutput.row(index) = b.logoutput.row( index - ChunkSize/2 );
            ++index;
        }
    }

  private:
    JointData& d;
    DataRow* current;
    int index;
    mutable DataPointReference r;

    void next_chunk() {
        d.data.push_back( DataRow() ); 
        index = 0;
        current = &d.data.back();
    }

    friend class ::boost::iterator_core_access;
    void increment() {
        ++index;
    }

    DataPointReference& dereference() const { 
        if ( index == ChunkSize ) 
            const_cast<data_point_iterator&>(*this).next_chunk();
        return (r = DataPointReference(current,index)); 
    }

};

template <typename Num,typename LengthUnit, int ChunkSize>
typename JointData<Num,LengthUnit,ChunkSize>::data_point_iterator
JointData<Num,LengthUnit,ChunkSize>::point_back_inserter()
{
    return data_point_iterator( *this );
}

template <typename Num, typename LengthUnit, int ChunkSize>
template <typename ONum, int Width>
JointData<Num,LengthUnit,ChunkSize>::JointData
    ( const DisjointData<ONum,LengthUnit, Width>& od )
: GenericData<LengthUnit>( od )
{
    typedef DisjointData<ONum,LengthUnit, Width> Source;
    data_point_iterator o = point_back_inserter();
    for (typename Source::const_iterator i = od.begin(); i != od.end(); ++i)
        for (int x = 0; x < Width; ++x)
            *o++ = DataPoint( od.xs[x], i->inputs[0], i->output[x] );
    o.pad_last_chunk();
}

template <typename Num, typename LengthUnit, int ChunkSize>
template <typename ONum, int Width>
JointData<Num,LengthUnit,ChunkSize>::JointData
    ( const JointData<ONum,LengthUnit,Width>& od )
: GenericData<LengthUnit>( od )
{
    const int Blocks = Width / ChunkSize;
    BOOST_STATIC_ASSERT(( Width % ChunkSize == 0 ));
    typedef JointData<ONum,LengthUnit,Width> Source;
    for (typename Source::const_iterator i = od.begin(); i != od.end(); ++i) {
        for ( int block = 0; block < Blocks; ++block ) {
            DataRow row;
            row.inputs = i->inputs.template block<ChunkSize,2>( block * ChunkSize, 0 );
            row.output = i->output.template block<ChunkSize,1>( block * ChunkSize, 0 );
            row.logoutput = i->logoutput.template block<ChunkSize,1>( block * ChunkSize, 0 );
            data.push_back( row );
        }
    }
}

}
}

#endif
