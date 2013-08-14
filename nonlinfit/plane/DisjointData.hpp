#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_DISJOINT_DATA_IMPL_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_DISJOINT_DATA_IMPL_H

#include "DisjointData.h"
#include "DataFacade.hpp"

namespace nonlinfit {
namespace plane {

template <typename Num, int ChunkSize>
DataPoint<Num>
DisjointCoreData<Num,ChunkSize>::get( const DataRow& chunk, int in_chunk ) const
{
    return data_point( 
        xs[in_chunk],
        chunk.inputs(0,0),
        chunk.output[in_chunk],
        chunk.logoutput[in_chunk],
        chunk.residues[in_chunk]);
}

template <typename Num, int ChunkSize>
void DisjointCoreData<Num,ChunkSize>::set( DataRow& chunk, int current_point, const data_point& point )
{
    chunk.inputs(0,0) = point.y();
    chunk.output[ current_point ] = point.value();
    chunk.logoutput[ current_point ] = point.logoutput();
    chunk.residues[ current_point ] = point.residue();
    xs[ current_point ] = point.x();
}

}
}

#endif
