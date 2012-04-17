#include "debug.h"
#include "SpotFinder.h"
#include <dStorm/Config.h>
#include <dStorm/traits/ScaledProjection.h>
#include <boost/units/cmath.hpp>

using namespace std;

namespace dStorm {
namespace engine {
namespace spot_finder {

boost::units::quantity< boost::units::camera::length > Job::sigma(int dim) const
{
    return dynamic_cast< const traits::ScaledProjection& >(traits.projection())
        .length_in_image_space(dim, 
        (*traits.optics.psf_size(fluorophore.ident))[dim] );
}

boost::units::quantity< boost::units::camera::length, int > Job::mask_size(int dim) const
{
    return smoothing_mask;
}

ImageTypes<2>::Size Job::size() const
{
    return traits.image.size;
}

Base::Base(const Job& job)
:    smoothed( job.size(), 0 * camera::frame )
     {
        DEBUG("Making SpotFinder with " << msx << " " << msy );
        DEBUG("Image pointer is " << smoothed.ptr() );
        /* Zero the border of the smoothed image to be sure no maximums
         * occur here regardless of mask size combinations. */
        smoothed.fill(0);
     }

Base::~Base() {
    DEBUG( "Destructing SpotFinder" );
}

void Base::findCandidates( Candidates& into ) {
    into.fill( getSmoothedImage() );
}

}
}
}
