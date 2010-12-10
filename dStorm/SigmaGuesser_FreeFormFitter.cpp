#include "SigmaFitter.h"
#include <CImgBuffer/Image.h>
#include <CImg.h>

using namespace std;
using namespace libfitpp;

namespace dStorm {

SigmaFitter::SigmaFitter(Config &config) throw() {
    fitter.reset(new Exponential2D_FixedCenter<StormPixel>
        (config.delta_sigma()/5));
}

void SigmaFitter::useConfig(Config &config) throw() {
    initial_sigmas[0] = config.sigma_x();
    initial_sigmas[1] = config.sigma_y();
    initial_sigmas[2] = config.sigma_xy();

    prefac = 2 * M_PI * config.sigma_x() * config.sigma_y() *
                  sqrt( 1 - config.sigma_xy() * config.sigma_xy() );
    /* This parameter is set independently of the fit mask size because
     * small fit masks have a very detrimental effect on free-form fitting. */
    int nmsx = int((3*config.sigma_x())) * 2 + 1;
    int nmsy = int((3*config.sigma_y())) * 2 + 1;
    if (fitter.get() == NULL || nmsx != msx || nmsy != msy) {
        msx = nmsx; msy = nmsy;
        fitter->setSize( msx, msy );
    }
}

static const double sigmaTol = 2;

bool SigmaFitter::fit(const cimg_library::CImg<StormPixel> &i,
    const Localization &f, double dev[4]) 
    throw()
{
    fitter->setData(i.ptr(), 1, i.width, 0, i.width-1, 0, i.height-1);
    
    fitter->setMeanX(f.getPreciseX());
    fitter->setMeanY(f.getPreciseY());
    fitter->setSigmaX(initial_sigmas[0]);
    fitter->setSigmaY(initial_sigmas[1]);
    fitter->setSigmaXY(initial_sigmas[2]);
    fitter->setShift( fitter->getCorner(f.x(1), f.y(1), -1, -1) );
    fitter->setAmplitude( f.getStrength() );
    fitter->fit();

    double amp = abs(fitter->getAmplitude()),
           nsigmaX = abs(fitter->getSigmaX()),
           nsigmaY = abs(fitter->getSigmaY());
    dev[0] = nsigmaX;
    dev[1] = nsigmaY;
    dev[2] = amp;
    dev[3] = fitter->getSigmaXY();
    if (amp < f.getStrength() / 10 || amp > f.getStrength() * 10
        || (nsigmaX < initial_sigmas[0] / sigmaTol) 
        || (nsigmaX > initial_sigmas[0] * sigmaTol) 
        || (nsigmaY < initial_sigmas[1] / sigmaTol) 
        || (nsigmaY > initial_sigmas[1] * sigmaTol)
        || (fitter->getSigmaXY() > 1) 
        || (fitter->getSigmaXY() < -1) )
        return false;
    else {
        return true;
    }
}

}
