#include "SpotFinder.h"
#include "Config.h"
#include <CImg.h>
#include <dStorm/helpers/thread.h>

using namespace std;
using namespace cimg_library;

namespace dStorm {
namespace engine {

SpotFinder::SpotFinder(const Config &c, 
                       pixel_count imw, pixel_count imh)
   : msx(c.x_maskSize()), msy(c.y_maskSize()),
     bx(c.fitWidth()), by(c.fitHeight()),
     imw(imw.value()), imh(imh.value()),
     smoothed( new CImg<SmoothedPixel>(imw.value(), 
                                       imh.value()) )
     {
        PROGRESS("Making SpotFinder with " 
                 << msx << " " << msy << " " << imw << " " << imh);
        PROGRESS("Image pointer is " << smoothed->ptr() );
        /* Zero the border of the smoothed image to be sure no maximums
         * occur here regardless of mask size combinations. */
        memset( smoothed->ptr(), 0, smoothed->size() * sizeof(SmoothedPixel) );
     }

SpotFinder::~SpotFinder() {
    PROGRESS( "Destructing SpotFinder" );
}

void SpotFinder::findCandidates( Candidates& into ) {
    into.fill( getSmoothedImage() );
}

}
}
