#include "GaussFitter.h"
#include <stdio.h>
#include <cassert>
#include <limits>
#include <cc++/thread.h>
#include "ImageFitter.h"

using namespace dStorm;
using namespace std;
using namespace libfitpp;

#define SQ(x) ((x)*(x))

bool GaussFitter::fitSpot( const Spot &spot, Localization *target ) throw() 
{
   FitResult fitResult;

   double x_start = spot.x(), y_start = spot.y();

   fitResult = fitToImage(*image, fitter, x_start, y_start);
   assert( fitResult != TooCloseToImageBorder );

    double x_end = fitter.getMeanX(),
           y_end = fitter.getMeanY();
    double amp = fitter.getAmplitude();
    new(target) Localization( x_end, y_end, image->getImageNumber(), amp );
    bool is_good = 
        (/*fitSuccessful &&*/ amp > amplitude_threshold 
          && x_end >= 1 && y_end >= 1 
          && x_end <= image->width-2 && y_end <= image->height-2 
          && SQ(x_start - x_end) + SQ(y_start - y_end) < 4 );
    return is_good;
}

GaussFitter::GaussFitter(const Config &config) 
throw() 
: fitter( config.negligibleStepLength() ),
  config(config), msx(config.fitWidth()), msy(config.fitHeight()),
  image(NULL)
{
   fitter.setSigmaX( config.sigma_x() );
   fitter.setSigmaY( config.sigma_y() );
   fitter.setSigmaXY( config.sigma_xy() );

   fitter.setSize( msx, msy );

   fitter.setStartLambda( config.marquardtStartLambda() );
   fitter.setMaximumIterationSteps( config.maximumIterationSteps() );
   fitter.setSuccessiveNegligibleStepLimit( 
        config.successiveNegligibleSteps() );
   fitter.setLimitForNegligibleCorrelation( 
        config.sigma_xy_negligible_limit() );

   amplitude_threshold = config.amplitude_threshold();
   PROGRESS("Amplitude threshold is " << amplitude_threshold);
}

GaussFitter::~GaussFitter() throw() {
}

void GaussFitter::prepareToFitImage(const Image &im) {
    image = &im;
}
