#ifndef DSTORM_GUF_DATACUBE_IMPL_H
#define DSTORM_GUF_DATACUBE_IMPL_H

#include "FittingRegionStack.h"
#include <dStorm/image/slice.h>
#include <dStorm/engine/InputTraits.h>
#include <dStorm/engine/InputPlane.h>
#include <dStorm/engine/Image.h>
#include "Centroid.h"
#include "Optics.h"
#include "ScheduleIndexFinder.h"
#include "FittingRegionCreator.h"

namespace dStorm {
namespace fit_window {

class FittingRegionStackCreator::Plane {
private:
    Optics optics;
    ScheduleIndexFinder index_finder;
    FittingRegionCreatorTable extractor_table;

public:
    Plane( const Config& c, const engine::InputPlane& plane )
        : optics( Spot::Constant( Spot::Scalar( c.fit_window_size() ) ), plane ),
          index_finder( c.allow_disjoint(), c.double_computation(), optics ),
          extractor_table( optics )
        {}
    std::auto_ptr<FittingRegion> set_image( const engine::Image2D& image, const Spot& position ) const {
        int index = index_finder.get_evaluation_tag_index( position );
        std::auto_ptr<FittingRegion> rv = extractor_table.get( index ).extract_data( image, position );
        rv->tag_index = index;
        return rv;
    }
};

FittingRegionStackCreator::FittingRegionStackCreator( const Config& ad, const dStorm::engine::JobInfo& info )
{
    for (int i = 0; i < info.traits.plane_count(); ++i)
        planes.push_back( new Plane( ad, info.traits.plane(i) ) );
}

FittingRegionStackCreator::~FittingRegionStackCreator() {}

std::auto_ptr< FittingRegionStack >
FittingRegionStackCreator::set_image( 
    const dStorm::engine::ImageStack& image,
    const Spot& position ) const
{
    std::auto_ptr< FittingRegionStack > rv( new FittingRegionStack() );
    typename boost::ptr_vector< Plane >::const_iterator b, i, e = planes.end();
    for ( b = i = planes.begin(); i != e; ++i) {
        rv->planes.push_back( 
            i->set_image( image.plane(i - b), position ) );
    }
    return rv;
}

FittingRegionStack::FittingRegionStack() {}

Centroid FittingRegionStack::residue_centroid() const
{
    Centroid rv;
    for ( size_t i = 0; i < planes.size(); ++i )
        rv += *planes[i].residue_centroid();
    return rv;
}

}
}

#endif
