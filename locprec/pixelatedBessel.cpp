#include <gsl/gsl_integration.h>
#include <gsl/gsl_sf_bessel.h>

struct LineInfo {
    double y, correlation;
};

double point(double x, void *params) 
{
    const struct LineInfo &line = *(struct LineInfo*)params;
    double dist_sq, dist, bessel;
    const double &y = line.y;
  recompute:
    dist_sq = x*(x-line.correlation*y) + y*(y-line.correlation*x);
    if (dist_sq == 0) { x += 0.01; goto recompute; }
    dist = sqrt(dist_sq);
    bessel = gsl_sf_bessel_J1(dist);
    return (bessel / dist_sq) * bessel;
}

struct Scaled {
  double v, s, correlation;
};

double line(double y, void *params) 
{
    struct Scaled &xs = *(struct Scaled*)params;
    double& xc = xs.v, &xf = xs.s;
    double val = 0, abserr;
    size_t neval;

    struct LineInfo info;
    info.y = y;
    info.correlation = xs.correlation;

    gsl_function func;
    func.function = point;
    func.params = &info;

    gsl_integration_qng( &func, (xc-0.5)/xf, (xc+0.5)/xf, 1E-2, 0, 
                         &val, &abserr, &neval );

    return val;
}

double integratedBesselFunction(double x, double y, double xs, double ys,
                                double corr) 
{
    double val = -1, abserr;
    size_t neval;

    struct Scaled scaled;
    scaled.v = x;
    scaled.s = xs;
    scaled.correlation = (corr);

    gsl_function func;
    func.function = line;
    func.params = &scaled;

    gsl_integration_qng( &func, (y-0.5)/ys, (y+0.5)/ys,
                         1E-2, 0, &val, &abserr, &neval );
    return val;
}
