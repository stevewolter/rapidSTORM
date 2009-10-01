#include "engine/SpotFinder.h"
#include <foreach.h>
#include <CImg.h>
#include "spotFinders/averageSmooth.h"
#include <cc++/thread.h>

#include "spotFinders/Spalttiefpass.h"
#include "spotFinders/MedianSmoother.h"
#include "spotFinders/ErosionSmoother.h"
#include "spotFinders/FillholeSmoother.h"
#include "spotFinders/GaussSmoothing.h"

using namespace std;
using namespace cimg_library;

namespace dStorm {

SpotFinder::SpotFinder(const Config &c, int imw, int imh)
   : msx(c.x_maskSize()), msy(c.y_maskSize()),
     bx(c.fitWidth()), by(c.fitHeight()),
     imw(imw), imh(imh),
     smoothed( new CImg<SmoothedPixel>(imw, imh) )
     {
        PROGRESS("Making SpotFinder with " 
                 << msx << " " << msy << " " << imw << " " << imh);
        /* Zero the border of the smoothed image to be sure no maximums
         * occur here regardless of mask size combinations. */
        memset( smoothed->ptr(), 0, smoothed->size() * sizeof(SmoothedPixel) );
     }

SpotFinder::~SpotFinder() {}

auto_ptr<SpotFinder> SpotFinder::factory
    (const Config &conf, int imw, int imh)

{
    SpotFinder *construct;
    if (conf.spotFindingMethod() == Average)
        construct = new Spalttiefpass(conf, imw, imh);
    else if (conf.spotFindingMethod() == Median)
        construct = new MedianSmoother(conf, imw, imh);
    else if (conf.spotFindingMethod() == Erosion)
        construct = new ErosionSmoother(conf, imw, imh);
#ifdef D3_INTERNAL_VERSION
    else if (conf.spotFindingMethod() == Reconstruction)
        construct = new FillholeSmoother(conf, imw, imh);
#endif
    else if (conf.spotFindingMethod() == Gaussian)
        construct = new GaussSmoother(conf, imw, imh);
    else {
        cerr << "Warning: Invalid choice " 
                << conf.spotFindingMethod() << " for SpotFindingMethod."
                << " Using Spalttiefpass instead." << endl;
        construct = new Spalttiefpass(conf, imw, imh);
    }
    return auto_ptr<SpotFinder>(construct);
}

}
