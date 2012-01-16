#include "debug.h"
#include "SpotFinder.h"
#include <dStorm/Config.h>

using namespace std;

namespace dStorm {
namespace engine {
namespace spot_finder {

double na_scale_factor = 3.8317 * 2.35 / (2*2*1.61632);

boost::units::quantity< boost::units::camera::length > Job::sigma(int dim) const
{
    return optics.length_in_image_space(dim, 
        quantity<si::length,float>((*traits.plane(0).psf_size(0))[dim] * fluorophore.wavelength / 
            traits.fluorophores.find(0)->second.wavelength) );
}

Base::Base(const Job& job)
   : msx( job.sigma(0) * job.smoothing_mask / camera::pixel ),
     msy( job.sigma(1) * job.smoothing_mask / camera::pixel ),
     bx( job.sigma(0) * job.smoothing_mask/ camera::pixel),
     by( job.sigma(1) * job.smoothing_mask/ camera::pixel),
     imw(job.size.x().value()), imh(job.size.y().value()),
     smoothed( job.size.head<2>(), 0 * camera::frame )
     {
        DEBUG("Making SpotFinder with " 
                 << msx << " " << msy << " " << imw << " " << imh);
        DEBUG("Image pointer is " << smoothed.ptr() );
        /* Zero the border of the smoothed image to be sure no maximums
         * occur here regardless of mask size combinations. */
        memset( smoothed.ptr(), 0, smoothed.size_in_pixels() * sizeof(SmoothedPixel) );
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
