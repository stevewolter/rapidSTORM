#define DSTORM_GAUSSSMOOTHING_CPP
#include "spotFinders/GaussSmoothing.h"
#include <dStorm/engine/Config.h>

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
    const Config&, const engine::spot_finder::Job &job)
: Base(job), xkern(msx+1, 0), ykern(msy+1, 0)
{
    fillWithGauss(xkern.ptr(), msx+1, 
        job.sigma(0) / camera::pixel, 256);
    fillWithGauss(ykern.ptr(), msy+1, 
        job.sigma(1) / camera::pixel, 256);
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

void GaussSmoother::smooth( const engine::Image2D &in )
 
{
    /* Effective border width */
    int eby = max(0,by-msy);

    for (int x = 0; x < int(in.height().value()); x++)
        gsm_line( 
            in.ptr(x, 0), in.width().value(),
            msy, in.height().value(),
            smoothed.ptr(x, 0), ykern.ptr() );

    SmoothedPixel copy[smoothed.width().value()];
    for (int y = eby; y < int(smoothed.width_in_pixels() - eby); y++) {
        memcpy(copy, smoothed.ptr(0, y), 
               sizeof(SmoothedPixel) * smoothed.width_in_pixels());
        gsm_line( copy, 1, msx, smoothed.width_in_pixels(),
                               smoothed.ptr(0, y), xkern.ptr() );
    }
}

std::auto_ptr<engine::spot_finder::Factory> make_Gaussian() { 
    return std::auto_ptr<engine::spot_finder::Factory>(new GaussSmoother::Factory()); 
}

}
}
