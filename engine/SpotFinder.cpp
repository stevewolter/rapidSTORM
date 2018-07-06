#include "debug.h"
#include "engine/SpotFinder.h"
#include <boost/units/cmath.hpp>

using namespace std;

namespace dStorm {
namespace engine {
namespace spot_finder {

ImageTypes<2>::Size Job::size() const
{
    return traits.image.size;
}

Base::Base(const Job& job)
:    smoothed( job.size(), 0 * camera::frame )
     {
        DEBUG("Making SpotFinder with " << job.size().transpose());
        DEBUG("Image pointer is " << smoothed.ptr());
        /* Zero the border of the smoothed image to be sure no maximums
         * occur here regardless of mask size combinations. */
        smoothed.fill(0);
     }

Base::~Base() {
    DEBUG( "Destructing SpotFinder" );
}

void Base::findCandidates( Candidates& into ) {
    into.fill( smoothed );
}

}
}
}
