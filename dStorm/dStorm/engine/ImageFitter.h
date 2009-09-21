#ifndef DSTORM_IMAGEFITTER_H
#define DSTORM_IMAGEFITTER_H

#include <CImg.h>
#include <fit++/Exponential2D.hh>

namespace dStorm {

enum FitResult { FitSuccessful, TooCloseToImageBorder, FitFailed };

/** The fitToImage function fits a point \c xc / \c yc in the image
 *  \c image with the \c fitter exponential fitter. It estimates mean,
 *  shift and amplitude from the data and calls the fit() method. */
template <typename DataType, typename Fitter>
FitResult fitToImage(
    const cimg_library::CImg<DataType>& image, 
    Deriver& fitter)
{
    using namespace fitpp::Exponential2D;
    LOCKING("Setting initial guesses");
    const int mx = int(round( fitter.getMeanX() )),
              my = int(round( fitter.getMeanY() ));

    fitter.setData( image.ptr(), 1, image.width, 
        0, image.width-1, 0, image.height-1 );
    fitter.setCenter( mx, my );

    DataType center = image(mx,my);
    DataType outer = 0;
    for (int x = -1; x <= 1; x += 2)
      for (int y = -1; y <= 1; y += 2)
        outer += fitter.getCorner( mx, my, x, y );
    outer /= 4;

    fitter.setShift( outer );
    fitter.setAmplitude( max<double>(center - outer, 10)
        * 2 * M_PI * fitter.getSigmaX() * fitter.getSigmaY());

    LOCKING("Running fit now");
    if ( fitter.fit() == fitpp::FitSuccess ) {
        return FitSuccessful;
    } else
        return FitFailed;
}

}

#endif
