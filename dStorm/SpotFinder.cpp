#include "SpotFinder.h"
#include <foreach.h>
#include <CImg.h>
#include "averageSmooth.h"
#include "dilation.h"
#include <cc++/thread.h>

#include "Spalttiefpass.h"
#include "MedianSmoother.h"
#include "ErosionSmoother.h"
#include "FillholeSmoother.h"
#include "GaussSmoothing.h"

using namespace std;
using namespace cimg_library;

namespace dStorm {

SpotFinder::SpotFinder(const Config &c, int imw, int imh) throw()
   : msx(c.x_maskSize()), msy(c.y_maskSize()),
     bx(c.fitWidth()), by(c.fitHeight()),
     imw(imw), imh(imh),
     smoothed( new CImg<SmoothedPixel>(imw, imh) )
     {
        PROGRESS("Making SpotFinder with " 
                 << msx << " " << msy << " " << imw << " " << imh);
        /* Write the border of the smoothed image to be sure no maximums
         * occur here regardless of mask size combinations. */
        memset( smoothed->ptr(), 0, smoothed->size() * sizeof(SmoothedPixel) );
     }

auto_ptr<SpotFinder> SpotFinder::factory
    (const Config &conf, int imw, int imh)
    throw()
{
    SpotFinder *construct;
    if (conf.spotFindingMethod.getValue() == "Average")
        construct = new Spalttiefpass(conf, imw, imh);
    else if (conf.spotFindingMethod.getValue() == "Median")
        construct = new MedianSmoother(conf, imw, imh);
    else if (conf.spotFindingMethod.getValue() == "Erosion")
        construct = new ErosionSmoother(conf, imw, imh);
    else if (conf.spotFindingMethod.getValue() == "Reconstruction")
        construct = new FillholeSmoother(conf, imw, imh);
    else if (conf.spotFindingMethod.getValue() == "Gaussian")
        construct = new GaussSmoother(conf, imw, imh);
    else {
        cerr << "Warning: Invalid choice " 
                << conf.spotFindingMethod.getValue() << " for SpotFindingMethod."
                << " Using Spalttiefpass instead." << endl;
        construct = new Spalttiefpass(conf, imw, imh);
    }
    return auto_ptr<SpotFinder>(construct);
}

}
