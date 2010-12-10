#include "../locprec/pixelatedBessel.h"
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <fit++/Exponential2D.hh>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_statistics.h>

using namespace std;
using namespace libfitpp;

int main(int argc, char *argv[]) 
{
    gsl_rng *rng = gsl_rng_alloc(gsl_rng_mt19937);
    gsl_rng_set(rng, time(NULL));

    double args[3] = { 1, 1, 1 };
    for (int i = 0; i < 3; i++)
        if (argc > i+1) {
            args[i] = atof(argv[i+1]);
        }

    int xr = int(6 * args[0]), yr = int(6 * args[1]);
    double data[2*yr+1][2*xr+1];

    Exponential2D fitter( 2*xr+1, 2*yr+1, Exponential2D::Double, 0x1F);
    for (int y = 0; y < 2*yr+1; y++)
        fitter.setDataLine(y, data[y]);

    int n = 50;
    double delta_x[n], delta_y[n], sigma[n];
    for (int run = 0; run < n; run++) {
        fitter.setParameter(Exponential2D::MeanX, 
                                 floor(xr+0.5+gsl_rng_uniform(rng)));
        fitter.setParameter(Exponential2D::MeanY, 
                                 floor(yr+0.5+gsl_rng_uniform(rng)));
        fitter.setParameter(Exponential2D::SigmaX, 1.32 * args[0]);
        fitter.setParameter(Exponential2D::SigmaY, 1.32 * args[1]);
        fitter.setParameter(Exponential2D::Amplitude, args[2]);
        fitter.setParameter(Exponential2D::Shift, 0);

        double cx = gsl_rng_uniform(rng), cy = gsl_rng_uniform(rng);
        for (int x = -xr; x <= xr; x++) {
            for (int y = -yr; y <= yr; y++) {
                data[y+yr][x+xr] = args[2] * 
                    integratedBesselFunction(x-cx, y-cy, args[0], args[1]);
            }
        }

        fitter.fit();

        delta_x[run] = (fitter.getParameter(Exponential2D::MeanX)-xr)-cx;
        delta_y[run] = (fitter.getParameter(Exponential2D::MeanY)-yr)-cy;
        sigma[run] = 0.5 * 
            (  fabs(fitter.getParameter(Exponential2D::SigmaX))
             + fabs(fitter.getParameter(Exponential2D::SigmaY)));
    }

    cerr << "Input sigmas ";
    cout << args[0] << " " << args[1] << " ";
    cerr << "Standard deviation in X and Y direction: ";
    cout << sqrt(gsl_stats_variance(delta_x, 1, n)) << " "
         << sqrt(gsl_stats_variance(delta_y, 1, n)) << " ";
    cerr << "Sigma average and deviation ";
    cout << gsl_stats_mean(sigma, 1, n) << " "
         << sqrt(gsl_stats_variance(sigma, 1, n)) << endl;

    return 0;
}
