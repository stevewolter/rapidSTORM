#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_JOINT_DATA_IMPL_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_JOINT_DATA_IMPL_H

#include "JointData.h"
#include "DataFacade.hpp"

namespace nonlinfit {
namespace plane {

template <typename Num,typename LengthUnit, int ChunkSize>
void JointCoreData<Num,LengthUnit,ChunkSize>::set( DataRow& row, int current_point, const data_point& point ) {
    row.inputs( current_point, 0 ) = point.x().value();
    row.inputs( current_point, 1 ) = point.y().value();
    row.output[ current_point ] = point.value();
    row.logoutput[ current_point ] = (point.value() < 1E-10) ? -23*point.value() : point.value() * log(point.value()) ;
}

template <typename Num,typename LengthUnit, int ChunkSize>
DataPoint<LengthUnit,Num>
JointCoreData<Num,LengthUnit,ChunkSize>::get( const DataRow& chunk, int in_chunk ) const
{
    return data_point( 
            data_point::Length::from_value(chunk.inputs(in_chunk,0)),
            data_point::Length::from_value(chunk.inputs(in_chunk,1)),
            chunk.output[in_chunk] );
}

template <typename Num, typename LengthUnit, int ChunkSize>
template <typename ONum, int Width>
JointData<Num,LengthUnit,ChunkSize>::JointData
    ( const DisjointData<ONum,LengthUnit, Width>& od )
: GenericData<LengthUnit>( od )
{
    std::copy( od.begin(), od.end(), std::back_inserter(*this) );
    this->pad_last_chunk();
}

template <typename Num, typename LengthUnit, int ChunkSize>
template <typename ONum, int Width>
JointData<Num,LengthUnit,ChunkSize>::JointData
    ( const JointData<ONum,LengthUnit,Width>& od )
: GenericData<LengthUnit>( od )
{
    std::copy( od.begin(), od.end(), std::back_inserter(*this) );
    this->pad_last_chunk();
}

}
}

#endif
