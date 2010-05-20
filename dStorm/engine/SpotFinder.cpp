#include "SpotFinder.h"
#include "Config.h"
#include <CImg.h>
#include <dStorm/helpers/thread.h>

using namespace std;
using namespace cimg_library;

namespace dStorm {
namespace engine {

SpotFinder::SpotFinder(const Config &c, 
                       const InputTraits::Size& size)
   : msx(c.x_maskSize()), msy(c.y_maskSize()),
     bx(c.fitWidth()), by(c.fitHeight()),
     imw(size.x().value()), imh(size.y().value()),
     smoothed( new SmoothedImage(size, 0 * cs_units::camera::frame) )
     {
        PROGRESS("Making SpotFinder with " 
                 << msx << " " << msy << " " << imw << " " << imh);
        PROGRESS("Image pointer is " << smoothed->ptr() );
        /* Zero the border of the smoothed image to be sure no maximums
         * occur here regardless of mask size combinations. */
        memset( smoothed->ptr(), 0, smoothed->size() / cs_units::camera::pixel / cs_units::camera::pixel * sizeof(SmoothedPixel) );
     }

SpotFinder::~SpotFinder() {
    PROGRESS( "Destructing SpotFinder" );
}

void SpotFinder::findCandidates( Candidates& into ) {
    into.fill( getSmoothedImage() );
}

}
}
