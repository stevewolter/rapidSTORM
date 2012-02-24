#ifndef DSTORM_GUF_DATACUBE_IMPL_H
#define DSTORM_GUF_DATACUBE_IMPL_H

#include "DataCube.h"
#include <dStorm/image/slice.h>
#include <dStorm/engine/InputTraits.h>
#include <dStorm/engine/Image.h>
#include "Centroid.h"

namespace dStorm {
namespace guf {

InputCube::InputCube( const Config& ad, const dStorm::engine::JobInfo& info )
{
    for (int i = 0; i < info.traits.plane_count(); ++i)
        planes.push_back( new InputPlane( ad, info, i ) );
}

std::auto_ptr< DataCube >
InputCube::set_image( 
    const dStorm::engine::ImageStack& image,
    const guf::Spot& position ) const
{
    std::auto_ptr< DataCube > rv( new DataCube() );
    typename boost::ptr_vector< InputPlane >::const_iterator b, i, e = planes.end();
    for ( b = i = planes.begin(); i != e; ++i) {
        rv->planes.push_back( 
            i->set_image( image.plane(i - b), position ) );
    }
    return rv;
}

DataCube::DataCube() {}

guf::Statistics<3> 
DataCube::get_statistics() const
{
    guf::Statistics<3> rv;
    for (size_t i = 0; i < planes.size(); ++i)
        rv.push_back( planes[i].get_statistics() );
    return rv;
}

Centroid DataCube::residue_centroid() const
{
    Centroid rv;
    for ( size_t i = 0; i < planes.size(); ++i )
        rv += *planes[i].residue_centroid();
    return rv;
}

}
}

#endif
