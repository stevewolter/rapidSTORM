#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_DISJOINT_DATA_IMPL_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_DISJOINT_DATA_IMPL_H

#include "nonlinfit/plane/DisjointData.h"
#include "nonlinfit/plane/DataFacade.hpp"

namespace nonlinfit {
namespace plane {

template <typename Num, int ChunkSize>
void DisjointCoreData<Num,ChunkSize>::set( DataRow& chunk, int current_point, const data_point& point )
{
    chunk.inputs(0,0) = point.y();
    xs[ current_point ] = point.x();
}

}
}

#endif
