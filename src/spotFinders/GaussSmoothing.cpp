#define DSTORM_GAUSSSMOOTHING_CPP
#include "spotFinders/GaussSmoothing.h"
#include <dStorm/engine/Config.h>
#include <CImg.h>

using namespace std;
using namespace dStorm::engine;

namespace dStorm {
namespace spotFinders {

static void fillWithGauss(int *values, int n, double sigma, int A) {
    const double sig_sq = sigma * sigma, norm = 1 / (2 * M_PI * sigma);

    for (int i = 0; i < n; i++)
        values[i] = (int)round(A * norm * exp( -0.5 * i * i / sig_sq ) );
}

GaussSmoother::GaussSmoother (
    const Config&, const engine::Config &conf, 
    const engine::Traits::Size& size )
: SpotFinder(conf, size), xkern(msx+1, 0), ykern(msy+1, 0)
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

#if cimg_version <= 129
#define CIMG_PAR
#else
#define CIMG_PAR ()
#endif
void GaussSmoother::smooth( const Image &in )
 
{
    /* Effective border width */
    int eby = max(0,by-msy);

    for (int x = 0; x < int(in.height CIMG_PAR); x++)
        gsm_line( &in(x, 0), in.width CIMG_PAR, msy, in.height CIMG_PAR, 
                  &(*smoothed)(x, 0), ykern.ptr() );

    SmoothedPixel copy[smoothed->width CIMG_PAR];
    for (int y = eby; y < int(smoothed->width CIMG_PAR - eby); y++) {
        memcpy(copy, &(*smoothed)(0, y), 
               sizeof(SmoothedPixel) * smoothed->width CIMG_PAR);
        gsm_line( copy, 1, msx, smoothed->width CIMG_PAR,
                               &(*smoothed)(0, y), xkern.ptr() );
    }
}

}
}
