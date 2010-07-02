#ifdef NDEBUG
#undef NDEBUG
#endif
#include "debug.h"
#include <Eigen/Array>
#include "fit++/Exponential2D.hh"
#include "fit++/Exponential2D_Uncorrelated_Derivatives.hh"
#include "fit++/Exponential2D_Correlated_Derivatives.hh"

using namespace fitpp::Exponential2D;

template <int FitFlags, bool Correlated, int Kernels, bool DynamicSize>
struct Checker {

static const int Width = (DynamicSize) ? Eigen::Dynamic : 9;
static const int Height = (DynamicSize) ? Eigen::Dynamic : 11;

typedef Model< Kernels, FitFlags > Space;
typedef Deriver< Kernels, FitFlags, Width, Height, Correlated > ToTest;
typedef Eigen::Matrix<double,Height,Width> Data;

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

    double 
        x0k0 = parameters.template getMeanX<0>(),
        y0k0 = parameters.template getMeanY<0>(),
        sxk0 = parameters.template getSigmaX<0>(),
        syk0 = parameters.template getSigmaY<0>(),
        Ak0 = parameters.template getAmplitude<0>(),
        rhok0 = (Correlated) ? parameters.template getSigmaXY<0>() : 0,
        x0k1 = (Space::Kernels >= 2) ? parameters.template getMeanX<1>() : 0,
        y0k1 = (Space::Kernels >= 2) ? parameters.template getMeanY<1>() : 0,
        sxk1 = (Space::Kernels >= 2) ? parameters.template getSigmaX<1>() : 1,
        syk1 = (Space::Kernels >= 2) ? parameters.template getSigmaY<1>() : 1,
        Ak1 = (Space::Kernels >= 2) ? parameters.template getAmplitude<1>() : 0,
        rhok1 = (Space::Kernels >= 2 && Correlated) ? parameters.template getSigmaXY<1>() : 0,
        B = parameters.getShift(),
        Pi = M_PI;

    double deriv[Space::VarC], value;
    for (int row = 0; row < data.rows(); ++row)
      for (int col = 0; col < data.cols(); ++col)
      {
        double x = col, y = row;

value = ( exp( - ( 0.5 * ( pow(x - x0k0, 2) / pow(sxk0, 2) + pow(y - y0k0, 2) / pow(syk0, 2) - ( 2 * rhok0 * ( x - x0k0)  * ( y - y0k0) )  / ( sxk0 * syk0) ) )  / ( 1 - pow(rhok0, 2)) ) * Ak0)  / ( 2 * Pi * sxk0 * syk0 * sqrt(1 - pow(rhok0, 2)))  + ( exp( - ( 0.5 * ( pow(x - x0k1, 2) / pow(sxk1, 2) + pow(y - y0k1, 2) / pow(syk1, 2) - ( 2 * rhok1 * ( x - x0k1)  * ( y - y0k1) )  / ( sxk1 * syk1) ) )  / ( 1 - pow(rhok1, 2)) ) * Ak1)  / ( 2 * Pi * sxk1 * syk1 * sqrt(1 - pow(rhok1, 2)))  + B;
if ( Space::template Parameter<Shift>::Variable ) deriv[Space::template Parameter<Shift>::template InKernel<0>::N] = 1;
if ( Space::template Parameter<Amplitude>::Variable ) deriv[Space::template Parameter<Amplitude>::template InKernel<0>::N] = exp( - ( 0.5 * ( pow(x - x0k0, 2) / pow(sxk0, 2) + pow(y - y0k0, 2) / pow(syk0, 2) - ( 2 * rhok0 * ( x - x0k0)  * ( y - y0k0) )  / ( sxk0 * syk0) ) )  / ( 1 - pow(rhok0, 2)) ) / ( 2 * Pi * sxk0 * syk0 * sqrt(1 - pow(rhok0, 2))) ;
if ( Space::template Parameter<MeanX>::Variable ) deriv[Space::template Parameter<MeanX>::template InKernel<0>::N] = (  - ( Ak0 * exp( - ( 0.5 * ( pow(x - x0k0, 2) / pow(sxk0, 2) + pow(y - y0k0, 2) / pow(syk0, 2) - ( 2 * rhok0 * ( x - x0k0)  * ( y - y0k0) )  / ( sxk0 * syk0) ) )  / ( 1 - pow(rhok0, 2)) ) * 0.5 * ( ( -2 * ( x - x0k0) )  / pow(sxk0, 2) - ( -2 * rhok0 * ( y - y0k0) )  / ( sxk0 * syk0) ) )  / ( 1 - pow(rhok0, 2)) )  / ( 2 * Pi * sxk0 * syk0 * sqrt(1 - pow(rhok0, 2))) ;
if ( Space::template Parameter<MeanY>::Variable ) deriv[Space::template Parameter<MeanY>::template InKernel<0>::N] = (  - ( Ak0 * exp( - ( 0.5 * ( pow(x - x0k0, 2) / pow(sxk0, 2) + pow(y - y0k0, 2) / pow(syk0, 2) - ( 2 * rhok0 * ( x - x0k0)  * ( y - y0k0) )  / ( sxk0 * syk0) ) )  / ( 1 - pow(rhok0, 2)) ) * 0.5 * ( ( -2 * ( y - y0k0) )  / pow(syk0, 2) - (  - 2 * rhok0 * ( x - x0k0) )  / ( sxk0 * syk0) ) )  / ( 1 - pow(rhok0, 2)) )  / ( 2 * Pi * sxk0 * syk0 * sqrt(1 - pow(rhok0, 2))) ;
if ( Space::template Parameter<SigmaX>::Variable ) deriv[Space::template Parameter<SigmaX>::template InKernel<0>::N] = (  - ( ( 2 * Pi * sxk0 * syk0 * sqrt(1 - pow(rhok0, 2)) * Ak0 * exp( - ( 0.5 * ( pow(x - x0k0, 2) / pow(sxk0, 2) + pow(y - y0k0, 2) / pow(syk0, 2) - ( 2 * rhok0 * ( x - x0k0)  * ( y - y0k0) )  / ( sxk0 * syk0) ) )  / ( 1 - pow(rhok0, 2)) ) * 0.5 * ( (  - pow(x - x0k0, 2) * 2 * sxk0)  / pow(sxk0, 4) - (  - 2 * rhok0 * ( x - x0k0)  * ( y - y0k0)  * syk0)  / pow(sxk0 * syk0, 2)) )  / ( 1 - pow(rhok0, 2))  + exp( - ( 0.5 * ( pow(x - x0k0, 2) / pow(sxk0, 2) + pow(y - y0k0, 2) / pow(syk0, 2) - ( 2 * rhok0 * ( x - x0k0)  * ( y - y0k0) )  / ( sxk0 * syk0) ) )  / ( 1 - pow(rhok0, 2)) ) * Ak0 * 2 * Pi * syk0 * sqrt(1 - pow(rhok0, 2))) )  / pow(2 * Pi * sxk0 * syk0 * sqrt(1 - pow(rhok0, 2)), 2);
if ( Space::template Parameter<SigmaY>::Variable ) deriv[Space::template Parameter<SigmaY>::template InKernel<0>::N] = (  - ( ( 2 * Pi * sxk0 * syk0 * sqrt(1 - pow(rhok0, 2)) * Ak0 * exp( - ( 0.5 * ( pow(x - x0k0, 2) / pow(sxk0, 2) + pow(y - y0k0, 2) / pow(syk0, 2) - ( 2 * rhok0 * ( x - x0k0)  * ( y - y0k0) )  / ( sxk0 * syk0) ) )  / ( 1 - pow(rhok0, 2)) ) * 0.5 * ( (  - pow(y - y0k0, 2) * 2 * syk0)  / pow(syk0, 4) - (  - 2 * rhok0 * ( x - x0k0)  * ( y - y0k0)  * sxk0)  / pow(sxk0 * syk0, 2)) )  / ( 1 - pow(rhok0, 2))  + exp( - ( 0.5 * ( pow(x - x0k0, 2) / pow(sxk0, 2) + pow(y - y0k0, 2) / pow(syk0, 2) - ( 2 * rhok0 * ( x - x0k0)  * ( y - y0k0) )  / ( sxk0 * syk0) ) )  / ( 1 - pow(rhok0, 2)) ) * Ak0 * 2 * Pi * sxk0 * sqrt(1 - pow(rhok0, 2))) )  / pow(2 * Pi * sxk0 * syk0 * sqrt(1 - pow(rhok0, 2)), 2);
if ( Space::template Parameter<SigmaXY>::Variable ) deriv[Space::template Parameter<SigmaXY>::template InKernel<0>::N] = ( ( exp( - ( 0.5 * ( pow(x - x0k0, 2) / pow(sxk0, 2) + pow(y - y0k0, 2) / pow(syk0, 2) - ( 2 * rhok0 * ( x - x0k0)  * ( y - y0k0) )  / ( sxk0 * syk0) ) )  / ( 1 - pow(rhok0, 2)) ) * Ak0 * 2 * Pi * sxk0 * syk0 * 2 * rhok0)  / ( 2 * sqrt(1 - pow(rhok0, 2)))  - ( 2 * Pi * sxk0 * syk0 * sqrt(1 - pow(rhok0, 2)) * Ak0 * exp( - ( 0.5 * ( pow(x - x0k0, 2) / pow(sxk0, 2) + pow(y - y0k0, 2) / pow(syk0, 2) - ( 2 * rhok0 * ( x - x0k0)  * ( y - y0k0) )  / ( sxk0 * syk0) ) )  / ( 1 - pow(rhok0, 2)) ) * ( ( ( 1 - pow(rhok0, 2))  * -0.5 * 2 * ( x - x0k0)  * ( y - y0k0) )  / ( sxk0 * syk0)  + 0.5 * ( pow(x - x0k0, 2) / pow(sxk0, 2) + pow(y - y0k0, 2) / pow(syk0, 2) - ( 2 * rhok0 * ( x - x0k0)  * ( y - y0k0) )  / ( sxk0 * syk0) )  * 2 * rhok0) )  / pow(1 - pow(rhok0, 2), 2))  / pow(2 * Pi * sxk0 * syk0 * sqrt(1 - pow(rhok0, 2)), 2);
if ( Kernels >= 2 ) {
if ( Space::template Parameter<Amplitude>::Variable ) deriv[Space::template Parameter<Amplitude>::template InKernel<1>::N] = exp( - ( 0.5 * ( pow(x - x0k1, 2) / pow(sxk1, 2) + pow(y - y0k1, 2) / pow(syk1, 2) - ( 2 * rhok1 * ( x - x0k1)  * ( y - y0k1) )  / ( sxk1 * syk1) ) )  / ( 1 - pow(rhok1, 2)) ) / ( 2 * Pi * sxk1 * syk1 * sqrt(1 - pow(rhok1, 2))) ;
if ( Space::template Parameter<MeanX>::Variable ) deriv[Space::template Parameter<MeanX>::template InKernel<1>::N] = (  - ( Ak1 * exp( - ( 0.5 * ( pow(x - x0k1, 2) / pow(sxk1, 2) + pow(y - y0k1, 2) / pow(syk1, 2) - ( 2 * rhok1 * ( x - x0k1)  * ( y - y0k1) )  / ( sxk1 * syk1) ) )  / ( 1 - pow(rhok1, 2)) ) * 0.5 * ( ( -2 * ( x - x0k1) )  / pow(sxk1, 2) - ( -2 * rhok1 * ( y - y0k1) )  / ( sxk1 * syk1) ) )  / ( 1 - pow(rhok1, 2)) )  / ( 2 * Pi * sxk1 * syk1 * sqrt(1 - pow(rhok1, 2))) ;
if ( Space::template Parameter<MeanY>::Variable ) deriv[Space::template Parameter<MeanY>::template InKernel<1>::N] = (  - ( Ak1 * exp( - ( 0.5 * ( pow(x - x0k1, 2) / pow(sxk1, 2) + pow(y - y0k1, 2) / pow(syk1, 2) - ( 2 * rhok1 * ( x - x0k1)  * ( y - y0k1) )  / ( sxk1 * syk1) ) )  / ( 1 - pow(rhok1, 2)) ) * 0.5 * ( ( -2 * ( y - y0k1) )  / pow(syk1, 2) - (  - 2 * rhok1 * ( x - x0k1) )  / ( sxk1 * syk1) ) )  / ( 1 - pow(rhok1, 2)) )  / ( 2 * Pi * sxk1 * syk1 * sqrt(1 - pow(rhok1, 2))) ;
if ( Space::template Parameter<SigmaX>::Variable ) deriv[Space::template Parameter<SigmaX>::template InKernel<1>::N] = (  - ( ( 2 * Pi * sxk1 * syk1 * sqrt(1 - pow(rhok1, 2)) * Ak1 * exp( - ( 0.5 * ( pow(x - x0k1, 2) / pow(sxk1, 2) + pow(y - y0k1, 2) / pow(syk1, 2) - ( 2 * rhok1 * ( x - x0k1)  * ( y - y0k1) )  / ( sxk1 * syk1) ) )  / ( 1 - pow(rhok1, 2)) ) * 0.5 * ( (  - pow(x - x0k1, 2) * 2 * sxk1)  / pow(sxk1, 4) - (  - 2 * rhok1 * ( x - x0k1)  * ( y - y0k1)  * syk1)  / pow(sxk1 * syk1, 2)) )  / ( 1 - pow(rhok1, 2))  + exp( - ( 0.5 * ( pow(x - x0k1, 2) / pow(sxk1, 2) + pow(y - y0k1, 2) / pow(syk1, 2) - ( 2 * rhok1 * ( x - x0k1)  * ( y - y0k1) )  / ( sxk1 * syk1) ) )  / ( 1 - pow(rhok1, 2)) ) * Ak1 * 2 * Pi * syk1 * sqrt(1 - pow(rhok1, 2))) )  / pow(2 * Pi * sxk1 * syk1 * sqrt(1 - pow(rhok1, 2)), 2);
if ( Space::template Parameter<SigmaY>::Variable ) deriv[Space::template Parameter<SigmaY>::template InKernel<1>::N] = (  - ( ( 2 * Pi * sxk1 * syk1 * sqrt(1 - pow(rhok1, 2)) * Ak1 * exp( - ( 0.5 * ( pow(x - x0k1, 2) / pow(sxk1, 2) + pow(y - y0k1, 2) / pow(syk1, 2) - ( 2 * rhok1 * ( x - x0k1)  * ( y - y0k1) )  / ( sxk1 * syk1) ) )  / ( 1 - pow(rhok1, 2)) ) * 0.5 * ( (  - pow(y - y0k1, 2) * 2 * syk1)  / pow(syk1, 4) - (  - 2 * rhok1 * ( x - x0k1)  * ( y - y0k1)  * sxk1)  / pow(sxk1 * syk1, 2)) )  / ( 1 - pow(rhok1, 2))  + exp( - ( 0.5 * ( pow(x - x0k1, 2) / pow(sxk1, 2) + pow(y - y0k1, 2) / pow(syk1, 2) - ( 2 * rhok1 * ( x - x0k1)  * ( y - y0k1) )  / ( sxk1 * syk1) ) )  / ( 1 - pow(rhok1, 2)) ) * Ak1 * 2 * Pi * sxk1 * sqrt(1 - pow(rhok1, 2))) )  / pow(2 * Pi * sxk1 * syk1 * sqrt(1 - pow(rhok1, 2)), 2);
if ( Space::template Parameter<SigmaXY>::Variable ) deriv[Space::template Parameter<SigmaXY>::template InKernel<1>::N] = ( ( exp( - ( 0.5 * ( pow(x - x0k1, 2) / pow(sxk1, 2) + pow(y - y0k1, 2) / pow(syk1, 2) - ( 2 * rhok1 * ( x - x0k1)  * ( y - y0k1) )  / ( sxk1 * syk1) ) )  / ( 1 - pow(rhok1, 2)) ) * Ak1 * 2 * Pi * sxk1 * syk1 * 2 * rhok1)  / ( 2 * sqrt(1 - pow(rhok1, 2)))  - ( 2 * Pi * sxk1 * syk1 * sqrt(1 - pow(rhok1, 2)) * Ak1 * exp( - ( 0.5 * ( pow(x - x0k1, 2) / pow(sxk1, 2) + pow(y - y0k1, 2) / pow(syk1, 2) - ( 2 * rhok1 * ( x - x0k1)  * ( y - y0k1) )  / ( sxk1 * syk1) ) )  / ( 1 - pow(rhok1, 2)) ) * ( ( ( 1 - pow(rhok1, 2))  * -0.5 * 2 * ( x - x0k1)  * ( y - y0k1) )  / ( sxk1 * syk1)  + 0.5 * ( pow(x - x0k1, 2) / pow(sxk1, 2) + pow(y - y0k1, 2) / pow(syk1, 2) - ( 2 * rhok1 * ( x - x0k1)  * ( y - y0k1) )  / ( sxk1 * syk1) )  * 2 * rhok1) )  / pow(1 - pow(rhok1, 2), 2))  / pow(2 * Pi * sxk1 * syk1 * sqrt(1 - pow(rhok1, 2)), 2);
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


static int check(double corr = 1) {
    typename Space::Variables vars;
    typename Space::Constants constants;
    Space parameters(&vars, &constants);
    Data data(11,9);

    parameters.template setMeanX<0>( 5.2  );
    parameters.template setMeanY<0>( 2.8  );
    parameters.template setAmplitude<0>( 2000 );
    parameters.template setSigmaX<0>( 2.8 );
    parameters.template setSigmaY<0>( 1.8 );
    parameters.template setSigmaXY<0>( 0.3 * corr );
    if ( Kernels >= 2 ) {
        parameters.template setMeanX<1>( 1.6  );
        parameters.template setMeanY<1>( 6.1  );
        parameters.template setAmplitude<1>( 2500 );
        parameters.template setSigmaX<1>( 1.8 );
        parameters.template setSigmaY<1>( 2.2 );
        parameters.template setSigmaXY<1>( -0.8 * corr );
    }
    parameters.template setShift( 15 );
    srand(50);
    for (int r = 0; r < data.rows(); ++r) 
      for (int c = 0; c < data.cols(); ++c) 
        data(r,c) = rand() * 1E-5;

    Data residues_naive(11,9), residues_optimized(11,9);
    typename Space::Vector gradient_naive, gradient_optimized;
    typename Space::Matrix hessian_naive, hessian_optimized;
    
    compute_residues_naively( parameters, data, residues_naive, gradient_naive, hessian_naive );

    ToTest test;
    if ( DynamicSize )
        test.resize( 9, 11 );
    test.prepare( vars, constants, 0, 0 );
    test.compute( data, residues_optimized, gradient_optimized, hessian_optimized );

#define CHECK(x,y) \
    if ( ( (x ## _naive - x ## _optimized).cwise().abs().cwise() > 1E-8 ).any() ) { \
        std::cerr << y << " for " << FitFlags << " " << Correlated << " " << Kernels << " did not match\n"; \
        std::cerr << x ## _naive << "\n\n" << x ## _optimized << "\n\n" << (x ## _naive - x ## _optimized) << std::endl; \
        std::cerr << std::endl; \
        return 1; \
    } else { \
        std::cerr << x ## _naive << "\n\n" << x ## _optimized << "\n\n" << (x ## _naive - x ## _optimized) << std::endl; \
    }
    CHECK(residues, "residue matrix");
    CHECK(gradient, "gradient");
    CHECK(hessian, "hessian");

    return 0;
}

};

int main() {
    return 
    Checker<FixedForm,false,1,false>::check() ||
    Checker<FixedForm,false,1,true>::check() ||
    Checker<FixedForm,false,2,false>::check() ||
    Checker<FixedForm,false,2,true>::check() ||
    Checker<FreeForm_NoCorrelation,false,1,false>::check() ||
    Checker<FreeForm_NoCorrelation,false,1,true>::check() ||
    Checker<FreeForm_NoCorrelation,false,2,false>::check() ||
    Checker<FreeForm_NoCorrelation,false,2,true>::check() ||
    Checker<FreeForm_NoCorrelation,true,1,false>::check() ||
    Checker<FreeForm_NoCorrelation,true,1,true>::check() ||
    Checker<FreeForm,true,1,false>::check( 1 ) ||
    Checker<FreeForm,true,1,true>::check( 1 ) ||
    Checker<FreeForm,true,1,false>::check( 0 ) ||
    Checker<FreeForm,true,1,true>::check( 0 ) ||
    Checker<FreeForm,true,2,false>::check() ||
    Checker<FreeForm,true,2,true>::check() ||
    Checker<FixedForm,true,1,false>::check() ||
    Checker<FixedForm,true,1,true>::check() ||
    Checker<FreeForm_NoCorrelation,true,2,false>::check() ||
    Checker<FreeForm_NoCorrelation,true,2,true>::check() ||
    Checker<FixedForm,true,2,false>::check() ||
    Checker<FixedForm,true,2,true>::check();
}
