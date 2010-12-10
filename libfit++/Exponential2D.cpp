#define DSTORM_EXPONENTIALFITFUNCTIONS_CPP
#include "Exponential2D.hh"
#include <math.h>
#include <stdarg.h>

using namespace std;
using namespace Eigen;

namespace libfitpp {

#define SQ(x) ((x)*(x))

template <typename DataPoint, int varC>
bool Exponential2D<DataPoint,varC>::computeDerivatives(
    double &residuesSq,
    typename FitFunction<varC>::Gradient& gradient,
    typename FitFunction<varC>::Curvature& redHess
) throw()
{
    /* Paramaters of the 2-D gauss function. */
    const double
        x0 = getParameter(MeanX), y0 = getParameter(MeanY), 
        sx = getParameter(SigmaX), sy = getParameter(SigmaY),
        amp = getParameter(Amplitude), b = getParameter(Shift),
        sxy = getParameter(SigmaXY);

    int xc = int(round(x0)), yc = int(round(y0));

    int x, y;
    const int xl = max(xc-this->xr, this->xs), 
              xh = min(xc+this->xr, this->xe);
    const int yl = max(yc-this->yr, this->ys), 
              yh = min(yc+this->yr, this->ye);

    if (sxy < -0.999 || sxy > 0.999 || sx < 0 || sy < 0 || amp < 0
        || x0 > xh || y0 > yh || x0 < xl || y0 < yl)
        return false;

    double ellip = (1 - sxy*sxy), ellipI = 1/ellip;
    bool correlationNegligible = 
        (abs(sxy) < correlationNegligibleLimit && !isVariable(SigmaXY));

    residuesSq = 0;
    gradient.setZero();
    if (!correlationNegligible) redHess.setZero();

    double sxI = 1/sx, syI = 1/sy;

    double x1s[xh-xl+1], x1sSq[xh-xl+1], Exs[xh-xl+1], Eys[yh-yl+1];
    for (x = 0; x <= xh-xl; x++) {
        x1s[x] = (x+xl-x0)*sxI;
        x1sSq[x] = SQ(x1s[x]);
        if (correlationNegligible)
            Exs[x] = exp(-0.5 * x1sSq[x]);
    }

    const double vf = 1 / (2 * M_PI * sx * sy * sqrt(ellip));

    for (y = yl; y <= yh; y++) {
        const double y1 = (y-y0) * syI, y1Sq = SQ(y1), 
            Ey = ((correlationNegligible) ? exp(-0.5 * y1Sq) : 0);
        Eys[y-yl] = Ey;
        for (x = xl; x <= xh; x++) {
            const double 
                x1 = x1s[x-xl], x1Sq = x1sSq[x-xl],
                covar = sxy * x1 * y1,
                E = ((correlationNegligible) ?
                      Exs[x-xl] * Ey :
                      exp( ellipI * (covar - 0.5*(x1Sq+y1Sq)) )),
                phi = amp * vf * E;

            /* The difference of our gauss result and the data. */
            double residues = this->getPoint(x,y) - (phi + b);

            residuesSq += residues * residues;

            /* The components of the gradient and Hessian matrices are 
             * the derivatives with respect to x0, y0, sigma and amp,
             * respectively. */
            Matrix<double,varC,1> deriv;
            for (int v = 0; v < varC; v++) {
                int p = this->variableIndex[v];

                switch (p) {
                  case MeanX:
                    deriv[v] = (phi*ellipI) * (x1 - sxy*y1) * sxI; break;
                  case MeanY:
                    deriv[v] = (phi*ellipI) * (y1 - sxy*x1) * syI; break;
                  case Amplitude:
                    deriv[v] = E * vf; break;
                  case SigmaX:
                    deriv[v] = ( phi*sxI ) * ( (x1Sq - covar)*ellipI -1 );
                    break;
                  case SigmaY:
                    deriv[v] = ( phi*syI ) * ( (y1Sq - covar)*ellipI -1 );
                    break;
                  case SigmaXY:
                    deriv[v] = (phi*ellipI) * ( 
                            (x1Sq/2 - covar + y1Sq/2)*ellipI - 
                                sxy*(1+ellipI) + x1*y1 );
                    break;
                  case Shift:
                    deriv[v] = 1;
                    break;
                }
            }

            gradient += residues * deriv;
            if (!correlationNegligible)
                redHess += deriv * deriv.transpose();
        }
    }

    /* The Hessian matrix can, alternatively, be computed with X and Y components
     * seperated. This approach is not, however, of much use; the gradient vector
     * cannot be seperated, necessitating a loop over all points and computing
     * derivatives there anyway. This approach gives numerical stability.  */
    if (correlationNegligible) {
        Matrix<double,varC,varC> xcontribs, ycontribs;
        Matrix<double,varC,1> deriv;

        xcontribs.setZero();
        ycontribs.setZero();

        for (y = yl; y <= yh; y++) {
            const double y1 = (y-y0) * syI, y1Sq = SQ(y1);
            double E = amp*vf * Eys[y-yl];
            for (int v = 0; v < varC; v++) {
                int p = this->variableIndex[v];
                
                switch (p) {
                  case MeanX:     deriv[v] = E; break;
                  case MeanY:     deriv[v] = E * y1 * syI; break;
                  case Amplitude: deriv[v] = Eys[y] * vf; break;
                  case SigmaX:    deriv[v] = E; break;
                  case SigmaY:    deriv[v] = E*syI * ( y1Sq -1 ); break;
                  default:
                  case Shift:     deriv[v] = 1; break;
                }

                for (int k = 0; k <= v; k++)
                    ycontribs(k,v) += deriv[v] * deriv[k];
            }
        }
        for (x = xl; x <= xh; x++) {
            const double x1 = (x-x0) * sxI, x1Sq = SQ(x1);
            double E = Exs[x-xl];
            for (int v = 0; v < varC; v++) {
                int p = this->variableIndex[v];
                
                switch (p) {
                  case MeanX:     deriv[v] = E * x1 * sxI; break;
                  case MeanY:     deriv[v] = E; break;
                  case Amplitude: deriv[v] = Exs[x]; break;
                  case SigmaX:    deriv[v] = E*sxI * ( x1Sq -1 ); break;
                  case SigmaY:    deriv[v] = E; break;
                  case SigmaXY:
                  case Shift:     deriv[v] = 1; break;
                }

                for (int k = 0; k <= v; k++)
                    xcontribs(k,v) += deriv[v] * deriv[k];
            }
        }

        redHess = xcontribs.cwise() * ycontribs;
    }

    return true;
}

template <typename DataPoint, int varC>
Exponential2D<DataPoint,varC>::Exponential2D() throw () 
: FitFunction<varC>(), LatticeFitFunction<DataPoint>()
{
    for (int param = LastParam; param >= FirstParam; param--)
        this->setVariableness(param, false);
    setParameter(SigmaXY, 0);

    correlationNegligibleLimit = 0.1;
}

template <typename DataPoint,int FitMask>
Exponential2D<DataPoint,FitMask>::~Exponential2D() throw() {}

// template class Exponential2D<unsigned short,1>;
// template class Exponential2D<float,1>;
// template class Exponential2D<double,1>;
template class Exponential2D<unsigned short,2>;
template class Exponential2D<float,2>;
template class Exponential2D<double,2>;
template class Exponential2D<unsigned short,3>;
template class Exponential2D<float,3>;
template class Exponential2D<double,3>;
template class Exponential2D<unsigned short,4>;
template class Exponential2D<float,4>;
template class Exponential2D<double,4>;
template class Exponential2D<unsigned short,5>;
template class Exponential2D<float,5>;
template class Exponential2D<double,5>;
template class Exponential2D<unsigned short,6>;
template class Exponential2D<float,6>;
template class Exponential2D<double,6>;
template class Exponential2D<unsigned short,7>;
template class Exponential2D<float,7>;
template class Exponential2D<double,7>;

}
