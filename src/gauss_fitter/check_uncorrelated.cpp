#include <Eigen/Array>
#include "fit++/Exponential2D.hh"
#include "fit++/Exponential2D_Uncorrelated_Derivatives.hh"
#include "fit++/Exponential2D_Correlated_Derivatives.hh"

using namespace fitpp::Exponential2D;

template <int FitFlags, bool Correlated>
struct Checker {

typedef Model< 1, FitFlags > Space;
typedef Deriver< 1, FitFlags, 9, 11, Correlated > ToTest;
typedef Eigen::Matrix<double,11,9> Data;

static void compute_residues_naively(
    const Space& parameters,
    const Data& data,
    Data& residues,
    typename Space::Vector& gradient,
    typename Space::Matrix& hessian
) {
    residues.fill(0);
    gradient.fill(0);
    hessian.fill(0);

    double x0 = parameters.template getMeanX<0>(),
           y0 = parameters.template getMeanY<0>(),
           sx = parameters.template getSigmaX<0>(),
           sy = parameters.template getSigmaY<0>(),
           A = parameters.template getAmplitude<0>(),
           B = parameters.getShift(),
           rho = (Correlated) ? parameters.template getSigmaXY<0>() : 0,
           Pi = M_PI;

    double deriv[Space::VarC], value;
    for (int row = 0; row < data.rows(); ++row)
      for (int col = 0; col < data.cols(); ++col)
      {
        double x = col, y = row;

value = ( exp( - ( 0.5 * ( pow(x - x0, 2) / pow(sx, 2) + pow(y - y0, 2) / pow(sy, 2) - ( 2 * rho * ( x - x0)  * ( y - y0) )  / ( sx * sy) ) )  / ( 1 - pow(rho, 2)) ) * A)  / ( 2 * Pi * sx * sy * sqrt(1 - pow(rho, 2)))  + B;
if ( Space::template Parameter<Shift>::Variable ) deriv[ Space::template Parameter<Shift>::template InKernel<0>::N ] = 1;
if ( Space::template Parameter<Amplitude>::Variable ) deriv[ Space::template Parameter<Amplitude>::template InKernel<0>::N ] = exp( - ( 0.5 * ( pow(x - x0, 2) / pow(sx, 2) + pow(y - y0, 2) / pow(sy, 2) - ( 2 * rho * ( x - x0)  * ( y - y0) )  / ( sx * sy) ) )  / ( 1 - pow(rho, 2)) ) / ( 2 * Pi * sx * sy * sqrt(1 - pow(rho, 2))) ;
if ( Space::template Parameter<MeanX>::Variable ) deriv[ Space::template Parameter<MeanX>::template InKernel<0>::N ] = (  - ( A * exp( - ( 0.5 * ( pow(x - x0, 2) / pow(sx, 2) + pow(y - y0, 2) / pow(sy, 2) - ( 2 * rho * ( x - x0)  * ( y - y0) )  / ( sx * sy) ) )  / ( 1 - pow(rho, 2)) ) * 0.5 * ( ( 2 * ( x - x0) )  / pow(sx, 2) - ( 2 * rho * ( y - y0) )  / ( sx * sy) ) )  / ( 1 - pow(rho, 2)) )  / ( 2 * Pi * sx * sy * sqrt(1 - pow(rho, 2))) ;
if ( Space::template Parameter<MeanY>::Variable ) deriv[ Space::template Parameter<MeanY>::template InKernel<0>::N ] = (  - ( A * exp( - ( 0.5 * ( pow(x - x0, 2) / pow(sx, 2) + pow(y - y0, 2) / pow(sy, 2) - ( 2 * rho * ( x - x0)  * ( y - y0) )  / ( sx * sy) ) )  / ( 1 - pow(rho, 2)) ) * 0.5 * ( ( 2 * ( y - y0) )  / pow(sy, 2) - ( 2 * rho * ( x - x0) )  / ( sx * sy) ) )  / ( 1 - pow(rho, 2)) )  / ( 2 * Pi * sx * sy * sqrt(1 - pow(rho, 2))) ;
if ( Space::template Parameter<SigmaX>::Variable ) deriv[ Space::template Parameter<SigmaX>::template InKernel<0>::N ] = (  - ( ( 2 * Pi * sx * sy * sqrt(1 - pow(rho, 2)) * A * exp( - ( 0.5 * ( pow(x - x0, 2) / pow(sx, 2) + pow(y - y0, 2) / pow(sy, 2) - ( 2 * rho * ( x - x0)  * ( y - y0) )  / ( sx * sy) ) )  / ( 1 - pow(rho, 2)) ) * 0.5 * ( (  - pow(x - x0, 2) * 2 * sx)  / pow(sx, 4) - (  - 2 * rho * ( x - x0)  * ( y - y0)  * sy)  / pow(sx * sy, 2)) )  / ( 1 - pow(rho, 2))  + exp( - ( 0.5 * ( pow(x - x0, 2) / pow(sx, 2) + pow(y - y0, 2) / pow(sy, 2) - ( 2 * rho * ( x - x0)  * ( y - y0) )  / ( sx * sy) ) )  / ( 1 - pow(rho, 2)) ) * A * 2 * Pi * sy * sqrt(1 - pow(rho, 2))) )  / pow(2 * Pi * sx * sy * sqrt(1 - pow(rho, 2)), 2);
if ( Space::template Parameter<SigmaY>::Variable ) deriv[ Space::template Parameter<SigmaY>::template InKernel<0>::N ] = (  - ( ( 2 * Pi * sx * sy * sqrt(1 - pow(rho, 2)) * A * exp( - ( 0.5 * ( pow(x - x0, 2) / pow(sx, 2) + pow(y - y0, 2) / pow(sy, 2) - ( 2 * rho * ( x - x0)  * ( y - y0) )  / ( sx * sy) ) )  / ( 1 - pow(rho, 2)) ) * 0.5 * ( (  - pow(y - y0, 2) * 2 * sy)  / pow(sy, 4) - (  - 2 * rho * ( x - x0)  * ( y - y0)  * sx)  / pow(sx * sy, 2)) )  / ( 1 - pow(rho, 2))  + exp( - ( 0.5 * ( pow(x - x0, 2) / pow(sx, 2) + pow(y - y0, 2) / pow(sy, 2) - ( 2 * rho * ( x - x0)  * ( y - y0) )  / ( sx * sy) ) )  / ( 1 - pow(rho, 2)) ) * A * 2 * Pi * sx * sqrt(1 - pow(rho, 2))) )  / pow(2 * Pi * sx * sy * sqrt(1 - pow(rho, 2)), 2);
if ( Space::template Parameter<SigmaXY>::Variable ) deriv[ Space::template Parameter<SigmaXY>::template InKernel<0>::N ] = ( ( exp( - ( 0.5 * ( pow(x - x0, 2) / pow(sx, 2) + pow(y - y0, 2) / pow(sy, 2) - ( 2 * rho * ( x - x0)  * ( y - y0) )  / ( sx * sy) ) )  / ( 1 - pow(rho, 2)) ) * A * 2 * Pi * sx * sy * 2 * rho)  / ( 2 * sqrt(1 - pow(rho, 2)))  - ( 2 * Pi * sx * sy * sqrt(1 - pow(rho, 2)) * A * exp( - ( 0.5 * ( pow(x - x0, 2) / pow(sx, 2) + pow(y - y0, 2) / pow(sy, 2) - ( 2 * rho * ( x - x0)  * ( y - y0) )  / ( sx * sy) ) )  / ( 1 - pow(rho, 2)) ) * ( ( ( 1 - pow(rho, 2))  * -0.5 * 2 * ( x - x0)  * ( y - y0) )  / ( sx * sy)  + 0.5 * ( pow(x - x0, 2) / pow(sx, 2) + pow(y - y0, 2) / pow(sy, 2) - ( 2 * rho * ( x - x0)  * ( y - y0) )  / ( sx * sy) )  * 2 * rho) )  / pow(1 - pow(rho, 2), 2))  / pow(2 * Pi * sx * sy * sqrt(1 - pow(rho, 2)), 2);
        
        residues(row,col) = data(row,col) - value;
        for (int i = 0; i < Space::VarC; ++i) {
            gradient[i] += residues(row,col) * deriv[i];
            for (int j = 0; j < Space::VarC; ++j) {
                hessian(i,j) += deriv[i] * deriv[j];
            }
        }
      }
}


static int check() {
    typename Space::Variables vars;
    typename Space::Constants constants;
    Space parameters(&vars, &constants);
    Data data;

    parameters.template setMeanX<0>( 5  );
    parameters.template setMeanY<0>( 2  );
    parameters.template setAmplitude<0>( 2000 );
    parameters.template setSigmaX<0>( 2.8 );
    parameters.template setSigmaY<0>( 1.8 );
    parameters.template setSigmaXY<0>( 0.3 );
    parameters.template setShift( 15 );
    srand(50);
    for (int r = 0; r < data.rows(); ++r) 
      for (int c = 0; c < data.cols(); ++c) 
        data(r,c) = 0 * rand() * 1E-5;

    Data residues_naive, residues_optimized;
    typename Space::Vector gradient_naive, gradient_optimized;
    typename Space::Matrix hessian_naive, hessian_optimized;
    
    compute_residues_naively( parameters, data, residues_naive, gradient_naive, hessian_naive );

    ToTest test;
    test.prepare( vars, constants, 0, 0 );
    test.compute( data, residues_optimized, gradient_optimized, hessian_optimized );

#define CHECK(x,y) \
    if ( ( (x ## _naive - x ## _optimized).cwise().abs().cwise() > 1E-8 ).any() ) { \
        std::cerr << y << " did not match\n"; \
        std::cerr << x ## _naive << "\n\n" << x ## _optimized << "\n\n" << (x ## _naive - x ## _optimized) << std::endl; \
        std::cerr << std::endl; \
        return 1; \
    }
    CHECK(residues, "residue matrix for " << FitFlags << " " << Correlated);
    CHECK(gradient, "gradient for " << FitFlags << " " << Correlated);
    CHECK(hessian, "hessian for " << FitFlags << " " << Correlated);

    return 0;
}

};

int main() {
    return 
    Checker<FreeForm_NoCorrelation,false>::check() ||
    Checker<FreeForm_NoCorrelation,true>::check() ||
    Checker<FreeForm,true>::check() ||
    Checker<FixedForm,false>::check() ||
    Checker<FixedForm,true>::check();
}
