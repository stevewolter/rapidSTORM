#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_JOINT_DATA_IMPL_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_JOINT_DATA_IMPL_H

#include "nonlinfit/plane/JointData.h"
#include "nonlinfit/plane/DataFacade.hpp"

namespace nonlinfit {
namespace plane {

template <typename Num, int ChunkSize>
void JointCoreData<Num,ChunkSize>::set( DataRow& row, int current_point, const data_point& point ) {
    row.inputs( current_point, 0 ) = point.x();
    row.inputs( current_point, 1 ) = point.y();
}

template <typename Num, int ChunkSize>
template <typename ONum, int Width>
JointData<Num,ChunkSize>::JointData( const DisjointData<ONum, Width>& od )
: GenericData( od )
{
    std::copy( od.begin(), od.end(), std::back_inserter(*this) );
    this->pad_last_chunk();
}

template <typename Num, int ChunkSize>
template <typename ONum, int Width>
JointData<Num,ChunkSize>::JointData( const JointData<ONum,Width>& od )
: GenericData( od )
{
    std::copy( od.begin(), od.end(), std::back_inserter(*this) );
    this->pad_last_chunk();
}

}
}

#endif
