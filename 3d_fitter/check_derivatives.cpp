#include "Exponential3D_Accessor.h"
#include "Exponential3D_Derivatives.h"

using namespace fitpp::Exponential3D;
using namespace boost::units;

template <int Kernels, int Widening>
struct Checker {

typedef Model< Kernels, Widening > Space;
typedef Deriver< Model<Kernels, Widening>, 9, 11 > ToTest;
typedef Eigen::Matrix<double,11,9> Data;

template <int Param,int Kernel>
struct Select {
    static const int N = (Space::template Parameter<Param>::Variable && Kernel < Kernels)
        ? Space::template Parameter<Param>::template InKernel<Kernel>::N
        : (Space::VarC);
};

static void compute_residues_naively(
    const typename Space::Accessor& parameters,
    const Data& data,
    Data& residues,
    typename Space::Vector& gradient,
    typename Space::Matrix& hessian
) {
    residues.fill(0);
    gradient.fill(0);
    hessian.fill(0);

    double
        x0k0 = parameters.template getMeanX<0>().value(),
        y0k0 = parameters.template getMeanY<0>().value(),
        zk0 = parameters.template getMeanZ<0>().value(),
        z0xk0 = parameters.template getZAtBestSigmaX<0>().value(),
        z0yk0 = parameters.template getZAtBestSigmaY<0>().value(),
        cxk0 = parameters.template getDeltaSigmaX<0>().value(),
        cyk0 = parameters.template getDeltaSigmaY<0>().value(),
        s0xk0 = parameters.template getBestSigmaX<0>().value(),
        s0yk0 = parameters.template getBestSigmaY<0>().value(),
        Ak0 = parameters.template getAmplitude<0>().value(),
        x0k1 = (Kernels >= 2) ? parameters.template getMeanX<1>().value() : 0,
        y0k1 = (Kernels >= 2) ? parameters.template getMeanY<1>().value() : 0,
        zk1 = (Kernels >= 2) ? parameters.template getMeanZ<1>().value() : 0,
        z0xk1 = (Kernels >= 2) ? parameters.template getZAtBestSigmaX<1>().value() : 0,
        z0yk1 = (Kernels >= 2) ? parameters.template getZAtBestSigmaY<1>().value() : 0,
        cxk1 = (Kernels >= 2) ? parameters.template getDeltaSigmaX<1>().value() : 1,
        cyk1 = (Kernels >= 2) ? parameters.template getDeltaSigmaY<1>().value() : 1,
        s0xk1 = (Kernels >= 2) ? parameters.template getBestSigmaX<1>().value() : 1,
        s0yk1 = (Kernels >= 2) ? parameters.template getBestSigmaY<1>().value() : 1,
        Ak1 = (Kernels >= 2) ? parameters.template getAmplitude<1>().value() : 0,
        B = parameters.template getShift().value(),
           Pi = M_PI;

    if ( Widening == Zhuang )  {
        cxk0 = sqrt(cxk0);
        cyk0 = sqrt(cyk0);
        cyk1 = sqrt(cyk1);
        cxk1 = sqrt(cxk1);
    }
    double deriv[Space::VarC+1], value;
    for (int row = 0; row < data.rows(); ++row)
      for (int col = 0; col < data.cols(); ++col)
      {
        double x = col, y = row;
        if ( Widening == Holtzer ) {
value = ( exp( - 0.5 * ( pow(x - x0k0, 2) / ( pow(cxk0 * ( zk0 - z0xk0) , 2) + pow(s0xk0, 2))  + pow(y - y0k0, 2) / ( pow(cyk0 * ( zk0 - z0yk0) , 2) + pow(s0yk0, 2)) ) ) * Ak0)  / ( 2 * Pi * sqrt(pow(cxk0 * ( zk0 - z0xk0) , 2) + pow(s0xk0, 2)) * sqrt(pow(cyk0 * ( zk0 - z0yk0) , 2) + pow(s0yk0, 2)))  + ( exp( - 0.5 * ( pow(x - x0k1, 2) / ( pow(cxk1 * ( zk1 - z0xk1) , 2) + pow(s0xk1, 2))  + pow(y - y0k1, 2) / ( pow(cyk1 * ( zk1 - z0yk1) , 2) + pow(s0yk1, 2)) ) ) * Ak1)  / ( 2 * Pi * sqrt(pow(cxk1 * ( zk1 - z0xk1) , 2) + pow(s0xk1, 2)) * sqrt(pow(cyk1 * ( zk1 - z0yk1) , 2) + pow(s0yk1, 2)))  + B;
deriv[Select<Shift,0>::N] = 1;
deriv[Select<Amplitude,0>::N] = exp( - 0.5 * ( pow(x - x0k0, 2) / ( pow(cxk0 * ( zk0 - z0xk0) , 2) + pow(s0xk0, 2))  + pow(y - y0k0, 2) / ( pow(cyk0 * ( zk0 - z0yk0) , 2) + pow(s0yk0, 2)) ) ) / ( 2 * Pi * sqrt(pow(cxk0 * ( zk0 - z0xk0) , 2) + pow(s0xk0, 2)) * sqrt(pow(cyk0 * ( zk0 - z0yk0) , 2) + pow(s0yk0, 2))) ;
deriv[Select<MeanX,0>::N] = (  - ( Ak0 * exp( - 0.5 * ( pow(x - x0k0, 2) / ( pow(cxk0 * ( zk0 - z0xk0) , 2) + pow(s0xk0, 2))  + pow(y - y0k0, 2) / ( pow(cyk0 * ( zk0 - z0yk0) , 2) + pow(s0yk0, 2)) ) ) * ( x0k0 - x) )  / ( pow(cxk0 * ( zk0 - z0xk0) , 2) + pow(s0xk0, 2)) )  / ( 2 * Pi * sqrt(pow(cxk0 * ( zk0 - z0xk0) , 2) + pow(s0xk0, 2)) * sqrt(pow(cyk0 * ( zk0 - z0yk0) , 2) + pow(s0yk0, 2))) ;
deriv[Select<MeanY,0>::N] = (  - ( Ak0 * exp( - 0.5 * ( pow(x - x0k0, 2) / ( pow(cxk0 * ( zk0 - z0xk0) , 2) + pow(s0xk0, 2))  + pow(y - y0k0, 2) / ( pow(cyk0 * ( zk0 - z0yk0) , 2) + pow(s0yk0, 2)) ) ) * ( y0k0 - y) )  / ( pow(cyk0 * ( zk0 - z0yk0) , 2) + pow(s0yk0, 2)) )  / ( 2 * Pi * sqrt(pow(cxk0 * ( zk0 - z0xk0) , 2) + pow(s0xk0, 2)) * sqrt(pow(cyk0 * ( zk0 - z0yk0) , 2) + pow(s0yk0, 2))) ;
deriv[Select<MeanZ,0>::N] = (  - ( 2 * Pi * sqrt(pow(cxk0 * ( zk0 - z0xk0) , 2) + pow(s0xk0, 2)) * sqrt(pow(cyk0 * ( zk0 - z0yk0) , 2) + pow(s0yk0, 2)) * 0.5 * ( (  - pow(y - y0k0, 2) * 2 * cyk0 * cyk0 * ( zk0 - z0yk0) )  / pow(pow(cyk0 * ( zk0 - z0yk0) , 2) + pow(s0yk0, 2), 2) - ( pow(x - x0k0, 2) * 2 * cxk0 * cxk0 * ( zk0 - z0xk0) )  / pow(pow(cxk0 * ( zk0 - z0xk0) , 2) + pow(s0xk0, 2), 2))  * exp( - 0.5 * ( pow(x - x0k0, 2) / ( pow(cxk0 * ( zk0 - z0xk0) , 2) + pow(s0xk0, 2))  + pow(y - y0k0, 2) / ( pow(cyk0 * ( zk0 - z0yk0) , 2) + pow(s0yk0, 2)) ) ) * Ak0 + exp( - 0.5 * ( pow(x - x0k0, 2) / ( pow(cxk0 * ( zk0 - z0xk0) , 2) + pow(s0xk0, 2))  + pow(y - y0k0, 2) / ( pow(cyk0 * ( zk0 - z0yk0) , 2) + pow(s0yk0, 2)) ) ) * Ak0 * ( ( 2 * Pi * sqrt(pow(cxk0 * ( zk0 - z0xk0) , 2) + pow(s0xk0, 2)) * 2 * cyk0 * cyk0 * ( zk0 - z0yk0) )  / ( 2 * sqrt(pow(cyk0 * ( zk0 - z0yk0) , 2) + pow(s0yk0, 2)))  + ( sqrt(pow(cyk0 * ( zk0 - z0yk0) , 2) + pow(s0yk0, 2)) * 2 * Pi * 2 * cxk0 * cxk0 * ( zk0 - z0xk0) )  / ( 2 * sqrt(pow(cxk0 * ( zk0 - z0xk0) , 2) + pow(s0xk0, 2))) ) ) )  / pow(2 * Pi * sqrt(pow(cxk0 * ( zk0 - z0xk0) , 2) + pow(s0xk0, 2)) * sqrt(pow(cyk0 * ( zk0 - z0yk0) , 2) + pow(s0yk0, 2)), 2);
deriv[Select<Amplitude,1>::N] = exp( - 0.5 * ( pow(x - x0k1, 2) / ( pow(cxk1 * ( zk1 - z0xk1) , 2) + pow(s0xk1, 2))  + pow(y - y0k1, 2) / ( pow(cyk1 * ( zk1 - z0yk1) , 2) + pow(s0yk1, 2)) ) ) / ( 2 * Pi * sqrt(pow(cxk1 * ( zk1 - z0xk1) , 2) + pow(s0xk1, 2)) * sqrt(pow(cyk1 * ( zk1 - z0yk1) , 2) + pow(s0yk1, 2))) ;
deriv[Select<MeanX,1>::N] = (  - ( Ak1 * exp( - 0.5 * ( pow(x - x0k1, 2) / ( pow(cxk1 * ( zk1 - z0xk1) , 2) + pow(s0xk1, 2))  + pow(y - y0k1, 2) / ( pow(cyk1 * ( zk1 - z0yk1) , 2) + pow(s0yk1, 2)) ) ) * ( x0k1 - x) )  / ( pow(cxk1 * ( zk1 - z0xk1) , 2) + pow(s0xk1, 2)) )  / ( 2 * Pi * sqrt(pow(cxk1 * ( zk1 - z0xk1) , 2) + pow(s0xk1, 2)) * sqrt(pow(cyk1 * ( zk1 - z0yk1) , 2) + pow(s0yk1, 2))) ;
deriv[Select<MeanY,1>::N] = (  - ( Ak1 * exp( - 0.5 * ( pow(x - x0k1, 2) / ( pow(cxk1 * ( zk1 - z0xk1) , 2) + pow(s0xk1, 2))  + pow(y - y0k1, 2) / ( pow(cyk1 * ( zk1 - z0yk1) , 2) + pow(s0yk1, 2)) ) ) * ( y0k1 - y) )  / ( pow(cyk1 * ( zk1 - z0yk1) , 2) + pow(s0yk1, 2)) )  / ( 2 * Pi * sqrt(pow(cxk1 * ( zk1 - z0xk1) , 2) + pow(s0xk1, 2)) * sqrt(pow(cyk1 * ( zk1 - z0yk1) , 2) + pow(s0yk1, 2))) ;
deriv[Select<MeanZ,1>::N] = (  - ( 2 * Pi * sqrt(pow(cxk1 * ( zk1 - z0xk1) , 2) + pow(s0xk1, 2)) * sqrt(pow(cyk1 * ( zk1 - z0yk1) , 2) + pow(s0yk1, 2)) * 0.5 * ( (  - pow(y - y0k1, 2) * 2 * cyk1 * cyk1 * ( zk1 - z0yk1) )  / pow(pow(cyk1 * ( zk1 - z0yk1) , 2) + pow(s0yk1, 2), 2) - ( pow(x - x0k1, 2) * 2 * cxk1 * cxk1 * ( zk1 - z0xk1) )  / pow(pow(cxk1 * ( zk1 - z0xk1) , 2) + pow(s0xk1, 2), 2))  * exp( - 0.5 * ( pow(x - x0k1, 2) / ( pow(cxk1 * ( zk1 - z0xk1) , 2) + pow(s0xk1, 2))  + pow(y - y0k1, 2) / ( pow(cyk1 * ( zk1 - z0yk1) , 2) + pow(s0yk1, 2)) ) ) * Ak1 + exp( - 0.5 * ( pow(x - x0k1, 2) / ( pow(cxk1 * ( zk1 - z0xk1) , 2) + pow(s0xk1, 2))  + pow(y - y0k1, 2) / ( pow(cyk1 * ( zk1 - z0yk1) , 2) + pow(s0yk1, 2)) ) ) * Ak1 * ( ( 2 * Pi * sqrt(pow(cxk1 * ( zk1 - z0xk1) , 2) + pow(s0xk1, 2)) * 2 * cyk1 * cyk1 * ( zk1 - z0yk1) )  / ( 2 * sqrt(pow(cyk1 * ( zk1 - z0yk1) , 2) + pow(s0yk1, 2)))  + ( sqrt(pow(cyk1 * ( zk1 - z0yk1) , 2) + pow(s0yk1, 2)) * 2 * Pi * 2 * cxk1 * cxk1 * ( zk1 - z0xk1) )  / ( 2 * sqrt(pow(cxk1 * ( zk1 - z0xk1) , 2) + pow(s0xk1, 2))) ) ) )  / pow(2 * Pi * sqrt(pow(cxk1 * ( zk1 - z0xk1) , 2) + pow(s0xk1, 2)) * sqrt(pow(cyk1 * ( zk1 - z0yk1) , 2) + pow(s0yk1, 2)), 2);
        } else {
value = ( exp( - 0.5 * ( pow(x - x0k0, 2) / pow(pow(cxk0 * ( zk0 - z0xk0) , 2) + s0xk0, 2) + pow(y - y0k0, 2) / pow(pow(cyk0 * ( zk0 - z0yk0) , 2) + s0yk0, 2)) ) * Ak0)  / ( 2 * Pi * ( pow(cxk0 * ( zk0 - z0xk0) , 2) + s0xk0)  * ( pow(cyk0 * ( zk0 - z0yk0) , 2) + s0yk0) )  + ( exp( - 0.5 * ( pow(x - x0k1, 2) / pow(pow(cxk1 * ( zk1 - z0xk1) , 2) + s0xk1, 2) + pow(y - y0k1, 2) / pow(pow(cyk1 * ( zk1 - z0yk1) , 2) + s0yk1, 2)) ) * Ak1)  / ( 2 * Pi * ( pow(cxk1 * ( zk1 - z0xk1) , 2) + s0xk1)  * ( pow(cyk1 * ( zk1 - z0yk1) , 2) + s0yk1) )  + B;
deriv[Select<Shift,0>::N] = 1;
deriv[Select<Amplitude,0>::N] = exp( - 0.5 * ( pow(x - x0k0, 2) / pow(pow(cxk0 * ( zk0 - z0xk0) , 2) + s0xk0, 2) + pow(y - y0k0, 2) / pow(pow(cyk0 * ( zk0 - z0yk0) , 2) + s0yk0, 2)) ) / ( 2 * Pi * ( pow(cxk0 * ( zk0 - z0xk0) , 2) + s0xk0)  * ( pow(cyk0 * ( zk0 - z0yk0) , 2) + s0yk0) ) ;
deriv[Select<MeanX,0>::N] = (  - ( Ak0 * exp( - 0.5 * ( pow(x - x0k0, 2) / pow(pow(cxk0 * ( zk0 - z0xk0) , 2) + s0xk0, 2) + pow(y - y0k0, 2) / pow(pow(cyk0 * ( zk0 - z0yk0) , 2) + s0yk0, 2)) ) * ( x0k0 - x) )  / pow(pow(cxk0 * ( zk0 - z0xk0) , 2) + s0xk0, 2))  / ( 2 * Pi * ( pow(cxk0 * ( zk0 - z0xk0) , 2) + s0xk0)  * ( pow(cyk0 * ( zk0 - z0yk0) , 2) + s0yk0) ) ;
deriv[Select<MeanY,0>::N] = (  - ( Ak0 * exp( - 0.5 * ( pow(x - x0k0, 2) / pow(pow(cxk0 * ( zk0 - z0xk0) , 2) + s0xk0, 2) + pow(y - y0k0, 2) / pow(pow(cyk0 * ( zk0 - z0yk0) , 2) + s0yk0, 2)) ) * ( y0k0 - y) )  / pow(pow(cyk0 * ( zk0 - z0yk0) , 2) + s0yk0, 2))  / ( 2 * Pi * ( pow(cxk0 * ( zk0 - z0xk0) , 2) + s0xk0)  * ( pow(cyk0 * ( zk0 - z0yk0) , 2) + s0yk0) ) ;
deriv[Select<MeanZ,0>::N] = (  - ( 2 * Pi * ( pow(cxk0 * ( zk0 - z0xk0) , 2) + s0xk0)  * ( pow(cyk0 * ( zk0 - z0yk0) , 2) + s0yk0)  * 0.5 * ( (  - pow(y - y0k0, 2) * 2 * 2 * cyk0 * cyk0 * ( zk0 - z0yk0)  * ( pow(cyk0 * ( zk0 - z0yk0) , 2) + s0yk0) )  / pow(pow(cyk0 * ( zk0 - z0yk0) , 2) + s0yk0, 4) - ( pow(x - x0k0, 2) * 2 * 2 * cxk0 * cxk0 * ( zk0 - z0xk0)  * ( pow(cxk0 * ( zk0 - z0xk0) , 2) + s0xk0) )  / pow(pow(cxk0 * ( zk0 - z0xk0) , 2) + s0xk0, 4))  * exp( - 0.5 * ( pow(x - x0k0, 2) / pow(pow(cxk0 * ( zk0 - z0xk0) , 2) + s0xk0, 2) + pow(y - y0k0, 2) / pow(pow(cyk0 * ( zk0 - z0yk0) , 2) + s0yk0, 2)) ) * Ak0 + exp( - 0.5 * ( pow(x - x0k0, 2) / pow(pow(cxk0 * ( zk0 - z0xk0) , 2) + s0xk0, 2) + pow(y - y0k0, 2) / pow(pow(cyk0 * ( zk0 - z0yk0) , 2) + s0yk0, 2)) ) * Ak0 * ( 2 * Pi * ( pow(cxk0 * ( zk0 - z0xk0) , 2) + s0xk0)  * 2 * cyk0 * cyk0 * ( zk0 - z0yk0)  + 2 * Pi * 2 * cxk0 * cxk0 * ( zk0 - z0xk0)  * ( pow(cyk0 * ( zk0 - z0yk0) , 2) + s0yk0) ) ) )  / pow(2 * Pi * ( pow(cxk0 * ( zk0 - z0xk0) , 2) + s0xk0)  * ( pow(cyk0 * ( zk0 - z0yk0) , 2) + s0yk0) , 2);
deriv[Select<Amplitude,1>::N] = exp( - 0.5 * ( pow(x - x0k1, 2) / pow(pow(cxk1 * ( zk1 - z0xk1) , 2) + s0xk1, 2) + pow(y - y0k1, 2) / pow(pow(cyk1 * ( zk1 - z0yk1) , 2) + s0yk1, 2)) ) / ( 2 * Pi * ( pow(cxk1 * ( zk1 - z0xk1) , 2) + s0xk1)  * ( pow(cyk1 * ( zk1 - z0yk1) , 2) + s0yk1) ) ;
deriv[Select<MeanX,1>::N] = (  - ( Ak1 * exp( - 0.5 * ( pow(x - x0k1, 2) / pow(pow(cxk1 * ( zk1 - z0xk1) , 2) + s0xk1, 2) + pow(y - y0k1, 2) / pow(pow(cyk1 * ( zk1 - z0yk1) , 2) + s0yk1, 2)) ) * ( x0k1 - x) )  / pow(pow(cxk1 * ( zk1 - z0xk1) , 2) + s0xk1, 2))  / ( 2 * Pi * ( pow(cxk1 * ( zk1 - z0xk1) , 2) + s0xk1)  * ( pow(cyk1 * ( zk1 - z0yk1) , 2) + s0yk1) ) ;
deriv[Select<MeanY,1>::N] = (  - ( Ak1 * exp( - 0.5 * ( pow(x - x0k1, 2) / pow(pow(cxk1 * ( zk1 - z0xk1) , 2) + s0xk1, 2) + pow(y - y0k1, 2) / pow(pow(cyk1 * ( zk1 - z0yk1) , 2) + s0yk1, 2)) ) * ( y0k1 - y) )  / pow(pow(cyk1 * ( zk1 - z0yk1) , 2) + s0yk1, 2))  / ( 2 * Pi * ( pow(cxk1 * ( zk1 - z0xk1) , 2) + s0xk1)  * ( pow(cyk1 * ( zk1 - z0yk1) , 2) + s0yk1) ) ;
deriv[Select<MeanZ,1>::N] = (  - ( 2 * Pi * ( pow(cxk1 * ( zk1 - z0xk1) , 2) + s0xk1)  * ( pow(cyk1 * ( zk1 - z0yk1) , 2) + s0yk1)  * 0.5 * ( (  - pow(y - y0k1, 2) * 2 * 2 * cyk1 * cyk1 * ( zk1 - z0yk1)  * ( pow(cyk1 * ( zk1 - z0yk1) , 2) + s0yk1) )  / pow(pow(cyk1 * ( zk1 - z0yk1) , 2) + s0yk1, 4) - ( pow(x - x0k1, 2) * 2 * 2 * cxk1 * cxk1 * ( zk1 - z0xk1)  * ( pow(cxk1 * ( zk1 - z0xk1) , 2) + s0xk1) )  / pow(pow(cxk1 * ( zk1 - z0xk1) , 2) + s0xk1, 4))  * exp( - 0.5 * ( pow(x - x0k1, 2) / pow(pow(cxk1 * ( zk1 - z0xk1) , 2) + s0xk1, 2) + pow(y - y0k1, 2) / pow(pow(cyk1 * ( zk1 - z0yk1) , 2) + s0yk1, 2)) ) * Ak1 + exp( - 0.5 * ( pow(x - x0k1, 2) / pow(pow(cxk1 * ( zk1 - z0xk1) , 2) + s0xk1, 2) + pow(y - y0k1, 2) / pow(pow(cyk1 * ( zk1 - z0yk1) , 2) + s0yk1, 2)) ) * Ak1 * ( 2 * Pi * ( pow(cxk1 * ( zk1 - z0xk1) , 2) + s0xk1)  * 2 * cyk1 * cyk1 * ( zk1 - z0yk1)  + 2 * Pi * 2 * cxk1 * cxk1 * ( zk1 - z0xk1)  * ( pow(cyk1 * ( zk1 - z0yk1) , 2) + s0yk1) ) ) )  / pow(2 * Pi * ( pow(cxk1 * ( zk1 - z0xk1) , 2) + s0xk1)  * ( pow(cyk1 * ( zk1 - z0yk1) , 2) + s0yk1) , 2);
        }
        
        residues(row,col) = data(row,col) - value;
        for (int i = 0; i < Space::VarC; ++i) {
            gradient[i] += residues(row,col) * deriv[i];
            for (int j = 0; j < Space::VarC; ++j) {
                hessian(i,j) += deriv[i] * deriv[j];
            }
        }
      }
}


static bool check() {
    typename ToTest::Space::Variables vars;
    typename ToTest::Space::Accessor parameters(&vars);
    Data data;

    parameters.template setMeanX<0>( 5 * camera::pixel );
    parameters.template setMeanY<0>( 2 * camera::pixel );
    parameters.template setMeanZ<0>( 35 * boost::units::si::nanometre );
    parameters.template setAmplitude<0>( 2000 * camera::ad_count );
    parameters.template setDeltaSigmaX<0>( Space::Accessor::QuantityDeltaSigmaX::from_value(0.02) );
    parameters.template setDeltaSigmaY<0>( Space::Accessor::QuantityDeltaSigmaY::from_value(0.03) );
    parameters.template setBestSigmaX<0>( 1.3 * camera::pixel );
    parameters.template setBestSigmaY<0>( 1.2 * camera::pixel );
    parameters.template setZAtBestSigmaX<0>( -100 * boost::units::si::nanometre );
    parameters.template setZAtBestSigmaY<0>( +200 * boost::units::si::nanometre );
    if ( Kernels >= 2 ) {
        parameters.template setMeanX<1>( 3 * camera::pixel );
        parameters.template setMeanY<1>( 6 * camera::pixel );
        parameters.template setMeanZ<1>( -65 * boost::units::si::nanometre );
        parameters.template setAmplitude<1>( 1000 * camera::ad_count );
        parameters.template setDeltaSigmaX<1>( Space::Accessor::QuantityDeltaSigmaX::from_value(0.01) );
        parameters.template setDeltaSigmaY<1>( Space::Accessor::QuantityDeltaSigmaY::from_value(0.04) );
        parameters.template setBestSigmaX<1>( 1.5 * camera::pixel );
        parameters.template setBestSigmaY<1>( 1.1 * camera::pixel );
        parameters.template setZAtBestSigmaX<1>( 200 * boost::units::si::nanometre );
        parameters.template setZAtBestSigmaY<1>( -300 * boost::units::si::nanometre );
    }
    parameters.template setShift( 15 * camera::ad_count );
    srand(50);
    for (int r = 0; r < data.rows(); ++r) 
      for (int c = 0; c < data.cols(); ++c) 
        data(r,c) = rand() * 1E-5;

    Data residues_naive, residues_optimized;
    typename Space::Vector gradient_naive, gradient_optimized;
    typename Space::Matrix hessian_naive, hessian_optimized;
    
    compute_residues_naively( parameters, data, residues_naive, gradient_naive, hessian_naive );

    ToTest test;
    test.prepare( vars, parameters.getConstants(), 0, 0, 0 );
    test.compute( data, residues_optimized, gradient_optimized, hessian_optimized );

#define CHECK(x) \
    if ( ( (x ## _naive - x ## _optimized).cwise().abs().cwise() > 1E-8 ).any() ) { \
        std::cerr << "Dismatch in " << Kernels << " " << Widening << std::endl; \
        std::cerr << x ## _naive << "\n\n" << x ## _optimized << "\n\n" << (x ## _naive - x ## _optimized) << std::endl; \
        std::cerr << std::endl; \
        return false; \
    }
    CHECK(residues);
    CHECK(gradient);
    CHECK(hessian);

    return true;
}

};

int main() {
    bool good = true;
    good = Checker<1,Holtzer>::check() && good;
    good = Checker<1,Zhuang>::check() && good;
    good = Checker<2,Holtzer>::check() && good;
    good = Checker<2,Zhuang>::check() && good;
    return ( good ) ? EXIT_SUCCESS : EXIT_FAILURE;
}
