#include "Fitter.h"
#include <fit++/FitFunction_impl.hh>
#include <fit++/Exponential2D_impl.hh>
#include <fit++/Exponential2D_Correlated_Derivatives.hh>

namespace Eigen {
    template <>
    class NumTraits<unsigned short>
        : public NumTraits<int> {};
}

using namespace std;
using namespace fitpp;

namespace dStorm {
namespace sigma_guesser {

Fitter::Fitter(const Config &config)
: msx(-1), msy(-1)
{
    fitter.reset( new _Fitter(constants)  );
    Fitting::Parameter<Exponential2D::SigmaX>::set_absolute_epsilon
        ( fit_function, config.delta_sigma()/15 );
    Fitting::Parameter<Exponential2D::SigmaY>::set_absolute_epsilon
        ( fit_function, config.delta_sigma()/15 );
    Fitting::Parameter<Exponential2D::SigmaXY>::set_absolute_epsilon
        ( fit_function, config.delta_sigma()/15 );

}

Fitter::~Fitter() {}

void Fitter::useConfig(const input::Traits<engine::Image>& traits) {
    double sigx = traits.psf_size().x() * traits.plane(0).resolution[0]->in_dpm() / camera::pixel;
    double sigy = traits.psf_size().y() * traits.plane(0).resolution[1]->in_dpm() / camera::pixel;
    initial_sigmas[0] = sigx;
    initial_sigmas[1] = sigy;
    initial_sigmas[2] = 0;

    prefac = 2 * M_PI * sigx * sigy;
    /* This parameter is set independently of the fit mask size because
     * small fit masks have a very detrimental effect on free-form fitting. */
    int nmsx = int((3*sigx));
    int nmsy = int((3*sigy));
    if (fitter.get() == NULL || nmsx != msx || nmsy != msy) {
        msx = nmsx; msy = nmsy;
        fitter->setSize( 2*msx+1, 2*msy+1 );
    }
}

static const double sigmaTol = 2;

bool Fitter::fit(const dStorm::Image<engine::StormPixel,2> &i,
    const Localization &f, double dev[4]) 

{
    double cx = f.position().x().value(), cy = f.position().y().value();
    int cxr = round(cx), cyr = round(cy);
    /* Reject localizations too close to image border. */
    if ( cxr < msx || cyr < msy 
         || cxr >= int(i.width_in_pixels()-msx) || cyr >= int(i.height_in_pixels()-msy) )
        return false;

    Fitting pos( &this->fitter->getVariables(), &constants );
    fitter->setData(i);
    fitter->setUpperLeftCorner( cxr-msx, cyr-msy );
    
    double start_amp =  f.amplitude() / camera::ad_counts;
    pos.setMeanX<0>(cx);
    pos.setMeanY<0>(cy);
    pos.setSigmaX<0>(initial_sigmas[0]);
    pos.setSigmaY<0>(initial_sigmas[1]);
    pos.setSigmaXY<0>(initial_sigmas[2]);
    pos.setShift( fitter->getCorner(-1, -1, 0) );
    pos.setAmplitude<0>( start_amp );
    fitter->fit( fit_function );

    pos.change_variable_set( &fitter->getVariables() );
    double amp = abs(pos.getAmplitude<0>()),
           nsigmaX = abs(pos.getSigmaX<0>()),
           nsigmaY = abs(pos.getSigmaY<0>());
    dev[0] = nsigmaX;
    dev[1] = nsigmaY;
    dev[2] = amp;
    dev[3] = 0;
    if (amp < start_amp / 10 || amp > start_amp * 10
        || (nsigmaX < initial_sigmas[0] / sigmaTol) 
        || (nsigmaX > initial_sigmas[0] * sigmaTol) 
        || (nsigmaY < initial_sigmas[1] / sigmaTol) 
        || (nsigmaY > initial_sigmas[1] * sigmaTol)
        || (pos.getSigmaXY<0>() > 1) 
        || (pos.getSigmaXY<0>() < -1) )
        return false;
    else {
        return true;
    }
}

}
}