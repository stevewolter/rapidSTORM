#include "debug.h"
#include "SpotFinder.h"
#include <dStorm/Config.h>
#include <dStorm/traits/ScaledProjection.h>

using namespace std;

namespace dStorm {
namespace engine {
namespace spot_finder {

double na_scale_factor = 3.8317 * 2.35 / (2*2*1.61632);

boost::units::quantity< boost::units::camera::length > Job::sigma(int dim) const
{
    return dynamic_cast< const traits::ScaledProjection& >(traits.projection())
        .length_in_image_space(dim, 
        (*traits.optics.psf_size(fluorophore.ident))[dim] );
}

Base::Base(const Job& job)
   : msx( job.mask_size(0) / camera::pixel ),
     msy( job.mask_size(1) / camera::pixel ),
     bx( msx ),
     by( msy ),
     smoothed( job.size(), 0 * camera::frame )
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
