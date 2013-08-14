#ifndef DSTORM_GUF_DATACUBE_HPP
#define DSTORM_GUF_DATACUBE_HPP

#include "Stack.h"
#include "Optics.h"
#include "PlaneCreator.hpp"
#include "ScheduleIndexFinder.hpp"
#include <dStorm/engine/InputTraits.h>

namespace dStorm {
namespace fit_window {

class StackCreator::Plane {
private:
    Optics optics;
    ScheduleIndexFinder index_finder;
    PlaneCreatorTable extractor_table;

public:
    template <typename Schedule>
    Plane( const Config& c, const engine::InputPlane& plane, Schedule s, int max_width )
        : optics( Spot::Constant( quantity<si::length>( c.fit_window_size() ).value() * 1E6 ), plane ),
          index_finder( s, c.allow_disjoint(), c.double_computation(), optics, max_width ),
          extractor_table( s, optics )
        {}
    std::auto_ptr<fit_window::Plane> set_image( const engine::Image2D& image, const Spot& position ) const; 
};

template <typename Schedule>
StackCreator::StackCreator( const Config& ad, const dStorm::engine::JobInfo& info, Schedule s, int max_width )
{
    for (int i = 0; i < info.traits.plane_count(); ++i)
        planes.push_back( new Plane( ad, info.traits.plane(i), s, max_width ) );
}


}
}

#endif
