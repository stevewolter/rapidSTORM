#ifndef DSTORM_GUF_DATACUBE_IMPL_H
#define DSTORM_GUF_DATACUBE_IMPL_H

#include "fit_window/Stack.hpp"
#include "image/slice.h"
#include "engine/InputPlane.h"
#include "engine/Image.h"

namespace dStorm {
namespace fit_window {

StackCreator::~StackCreator() {}

std::auto_ptr<fit_window::Plane>
StackCreator::Plane::set_image( const engine::Image2D& image, const Spot& position ) const
{
    int index = index_finder.get_evaluation_tag_index( position );
    std::auto_ptr<fit_window::Plane> rv = extractor_table.get( index ).extract_data( image, position );
    rv->tag_index = index;
    return rv;
}

std::auto_ptr< Stack >
StackCreator::set_image( 
    const dStorm::engine::ImageStack& image,
    const Spot& position ) const
{
    std::auto_ptr< Stack > rv( new Stack() );
    typename boost::ptr_vector< Plane >::const_iterator b, i, e = planes.end();
    for ( b = i = planes.begin(); i != e; ++i) {
        rv->planes.push_back( 
            i->set_image( image.plane(i - b), position ) );
    }
    return rv;
}

Stack::Stack() {}

Spot Stack::residue_centroid() const
{
    Spot rv = planes[0].residue_centroid();
    for ( size_t i = 1; i < planes.size(); ++i ) {
        Spot add = planes[i].residue_centroid();
        for (int dim = 0; dim < 2; ++dim) {
            rv[dim] = (add[dim] + rv[dim] * i) / (i+1);
        }
    }
    return rv;
}

}
}

#endif
