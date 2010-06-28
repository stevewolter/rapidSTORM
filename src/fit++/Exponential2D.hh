#ifndef LIBFITPP_EXPONENTIAL2D_H
#define LIBFITPP_EXPONENTIAL2D_H

#include <fit++/FitFunction.hh>
#include <fit++/LatticeFitFunction.hh>
#include "ParameterAllocator.hh"
#include "LeastSquaresLatticeFitter.hh"
#include "Exponential_Common_decl.h"

namespace fitpp {

/** The Exponential2D class fits the given lattice points with an
    *  exponential model of the form 
    *  A * exp( -1/2 * (x-x0)^2 / sigma_x^2 * (y-y0)^2 / sigma_y^2 ) + B.
    *
    *  All of these parameters must be declared as fixed or fitted
    *  at construction time. */
namespace Exponential2D {

using Exponential::MeanX;
using Exponential::MeanY;
using Exponential::Amplitude;
using Exponential::Shift;

static const int 
    SigmaX = 4, /**< sigma_x */ 
    SigmaY = 5, /**< sigma_y */ 
    SigmaXY = 6, /**< Correlation between axes */
    NumNames = 7,
    FunctionDeps = 6,
    Globals = 1;

static const int FixedForm =
    (1 << Shift) | (1 << MeanX) | (1 << MeanY) | (1 << Amplitude);
static const int FreeForm =
    FixedForm | (1 << SigmaX) | (1 << SigmaY) | (1 << SigmaXY);
static const int FreeForm_NoCorrelation =
    FixedForm | (1 << SigmaX) | (1 << SigmaY);
static const int FixedCenter =
    (1 << Shift) | (1 << Amplitude) |
    (1 << SigmaX) | (1 << SigmaY) | (1 << SigmaXY);

template <
    /** Number of Kernels of the exponential models */
    int Kernels,
    /** Bitfield indicating which parameters are 
     *  variables (1) or constants (0) */
    int ParameterMask,
    /** Width of the data raster */
    int Width, 
    /** Height of the data raster */
    int Height,
    /** Include correlation term between X and Y axis? */
    bool Correlation
>
class Deriver;

template <int Kernels, int ParameterMask> 
struct Model
: public ParamMap<Globals,Kernels,FunctionDeps,ParameterMask>
{

    typedef ParamMap<Globals,Kernels,FunctionDeps,ParameterMask>
        ParameterMap;

  public:
    static const int VarC = ParameterMap::VarC;
    static const int ConstC = ParameterMap::ConstC;
    typedef typename Eigen::Matrix<double,VarC,VarC> Matrix;
    typedef typename Eigen::Matrix<double,VarC,1> Vector;
    typedef typename ParameterMap::Variables Variables;
    typedef typename ParameterMap::Constants Constants;

  private:
    Variables *variables;
    Constants *constants;
  public:
    Model(Variables* variables, Constants *constants) 
        : variables(variables), constants(constants) {}

    void change_variable_set(Variables* variables) 
        { this->variables = (variables); }

    template <int name, int function>
    double get() const 
        {return ParameterMap::template value<name,function>
            (*variables, *constants);}
    template <int name, int function>
    void set(double value) 
        {ParameterMap::template value<name,function>
            (*variables, *constants) = value;}
    template <int name>
    void set_all(double value) 
        {ParameterMap::template Parameter<name>::set_all
            (*variables, *constants, value);}

#define METHODS(param) \
    template <int Function> \
    double get ## param() const { return get<param,Function>(); } \
    template <int Function> \
    void set ## param(double v) { set<param,Function>(v); } \
    void set_all_ ## param(double v) { set_all<param>(v); }
    METHODS(MeanX);
    METHODS(MeanY);
    METHODS(Amplitude);
    METHODS(SigmaX);
    METHODS(SigmaY);
    METHODS(SigmaXY);
#undef METHODS
    double getShift() const { return get<Shift,0>(); }
    void setShift(double v) { return set<Shift,0>(v); }

    template <int Function>
    Eigen::Vector2d getPosition() const {
        return Eigen::Vector2d( 
            getMeanX<Function>(), getMeanY<Function>() );
    }

    template <
            typename PixelType, 
            int Width = Eigen::Dynamic, int Height = Eigen::Dynamic,
            bool Use_Correlation = ParameterMap
                                ::template Parameter<SigmaXY>::Variable,
            bool Compute_Variances = false
        >
    struct Fitter
    {
        typedef LeastSquaresLatticeFitter<
            Model,
            Deriver<Kernels,ParameterMask,
                    Width,Height,Use_Correlation>,
            PixelType,
            Width, Height,
            Compute_Variances> Type;
    };
};

}
}
#endif
