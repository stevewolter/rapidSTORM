#define DSTORM_GAUSSSMOOTHING_CPP
#include "spotFinders/GaussSmoothing.h"
#include <dStorm/Config.h>
#include <CImg.h>

using namespace std;
using namespace dStorm;

static void fillWithGauss(int *values, int n, double sigma, int A) {
    const double sig_sq = sigma * sigma, norm = 1 / (2 * M_PI * sigma);

    for (int i = 0; i < n; i++)
        values[i] = (int)round(A * norm * exp( -0.5 * i * i / sig_sq ) );
}

GaussSmoother::GaussSmoother (const Config &conf, int imw, int imh)
 
: SpotFinder(conf, imw, imh), xkern(msx+1), ykern(msy+1)
{
    fillWithGauss(xkern.ptr(), msx+1, conf.sigma_x(), 256);
    fillWithGauss(ykern.ptr(), msy+1, conf.sigma_y(), 256);
}

template <typename InputPixel>
void gsm_line(const InputPixel *input, int step, int radius, int size,
          SmoothedPixel *target, int *weights)
{
    for (int c = radius; c < size - radius; c++) {
        int accum = input[c*step] * weights[0];
        for (int d = 1; d <= radius; d++) {
            accum += input[(c+d) * step] * weights[d];
            accum += input[(c-d) * step] * weights[d];
        }
        target[c*step] = accum;
    }
}

void GaussSmoother::smooth( const Image &in )
 
{
    /* Effective border width */
    int eby = max(0,by-msy);

    for (int x = 0; x < int(in.height); x++)
        gsm_line( in.ptr(x, 0), in.width, msy, in.height, 
                  smoothed->ptr(x, 0), ykern.ptr() );

    SmoothedPixel copy[smoothed->width];
    for (int y = eby; y < int(smoothed->width - eby); y++) {
        memcpy(copy, smoothed->ptr(0, y), 
               sizeof(SmoothedPixel) * smoothed->width);
        gsm_line( copy, 1, msx, smoothed->width,
                               smoothed->ptr(0, y), xkern.ptr() );
    }
}
