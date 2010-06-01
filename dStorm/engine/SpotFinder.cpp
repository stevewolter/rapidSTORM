#include "SpotFinder.h"
#include "Config.h"
#include <dStorm/helpers/thread.h>

using namespace std;

namespace dStorm {
namespace engine {

SpotFinder::SpotFinder(const Config &c, 
                       const InputTraits::Size& size)
   : msx(c.x_maskSize() / cs_units::camera::pixel),
     msy(c.y_maskSize() / cs_units::camera::pixel),
     bx(c.fitWidth() / cs_units::camera::pixel),
     by(c.fitHeight() / cs_units::camera::pixel),
     imw(size.x().value()), imh(size.y().value()),
     smoothed( new SmoothedImage(size, 0 * cs_units::camera::frame) )
     {
        PROGRESS("Making SpotFinder with " 
                 << msx << " " << msy << " " << imw << " " << imh);
        PROGRESS("Image pointer is " << smoothed->ptr() );
        /* Zero the border of the smoothed image to be sure no maximums
         * occur here regardless of mask size combinations. */
        memset( smoothed->ptr(), 0, smoothed->size_in_pixels() * sizeof(SmoothedPixel) );
     }

SpotFinder::~SpotFinder() {
    PROGRESS( "Destructing SpotFinder" );
}

void SpotFinder::findCandidates( Candidates& into ) {
    into.fill( getSmoothedImage() );
}

}
}
