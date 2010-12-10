#ifndef DSTORM_IMAGEFITTER_H
#define DSTORM_IMAGEFITTER_H

#include <CImg.h>
#include <fit++/Exponential2D.hh>

enum FitResult { FitSuccessful, TooCloseToImageBorder, FitFailed };
template <typename DataType, int ParamCount>
FitResult fitToImage(const cimg_library::CImg<DataType>& image, 
                libfitpp::Exponential2D<DataType,ParamCount>& fitter,
                double xc, double yc) throw()
{
    int mx = int(round(xc)), my = int(round(yc));

    fitter.setMeanX(xc);
    fitter.setMeanY(yc);
    fitter.setData( image.ptr(), 1, image.width, 
        0, image.width-1, 0, image.height-1 );

    DataType center = image(mx,my);
    DataType outer = 0;
    for (int x = -1; x <= 1; x += 2)
      for (int y = -1; y <= 1; y += 2)
        outer += fitter.getCorner( mx, my, x, y );
    outer /= 4;

    fitter.setShift( outer );
    fitter.setAmplitude( max<double>(center - outer, 10)
        * 2 * M_PI * fitter.getSigmaX() * fitter.getSigmaY());

    if ( fitter.fit() == libfitpp::FitFunction<ParamCount>::FitSuccess ) {
        return FitSuccessful;
    } else
        return FitFailed;
}

#endif
