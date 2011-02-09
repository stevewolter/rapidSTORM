#ifndef LIBFITPP_EXPONENTIAL3D_H
#define LIBFITPP_EXPONENTIAL3D_H

#include <fit++/FitFunction.hh>
#include <fit++/LatticeFitFunction.hh>
#include <fit++/ParameterAllocator.hh>
#include <fit++/LeastSquaresLatticeFitter.hh>
#include <fit++/Exponential_Common_decl.h>

namespace fitpp {

/** The Exponential3D class fits the given lattice points with an
    *  exponential model of the form 
    *  A * exp( -1/2 * (x-x0)^2 / sigma_x^2 * (y-y0)^2 / sigma_y^2 ) + B.
    *  where sigma_x = sqrt(delta_sigma_x * (z - zx0)^2 + sx0^2) and
    *  sigma_y accordingly.
    *
    *  All of these parameters must be declared as fixed or fitted
    *  at construction time. */
namespace Exponential3D {

using Exponential::MeanX;
using Exponential::MeanY;
using Exponential::Amplitude;
using Exponential::Shift;

static const int 
    MeanZ = 4,
    DeltaSigmaX = 5,
    DeltaSigmaY = 6,
    BestSigmaX = 7,
    BestSigmaY = 8,
    ZAtBestSigmaX = 9,
    ZAtBestSigmaY = 10,
    LayerDistance = 11,
    FunctionDeps = 11,
    Globals = 1;

enum Widenings {
    Holtzer,
    Zhuang
};

template <
    /** The model used. This class is not a subclass of model to allow 
     *  partial specialization. */
    class Model,
    /** Width of the data raster */
    int Width, 
    /** Height of the data raster */
    int Height
>
class Deriver;

template <int Kernels, int _Widening> 
struct Model
: public ParamMap<Globals,Kernels,FunctionDeps,(1 << (MeanZ+1)) - 1>
{

    typedef ParamMap<Globals,Kernels,FunctionDeps,(1 << (MeanZ+1)) - 1>
        ParameterMap;

  public:
    static const int VarC = ParameterMap::VarC;
    static const int ConstC = ParameterMap::ConstC;
    typedef typename Eigen::Matrix<double,VarC,VarC> Matrix;
    typedef typename Eigen::Matrix<double,VarC,1> Vector;
    typedef typename ParameterMap::Variables Variables;
    typedef typename ParameterMap::Constants Constants;

    static const int Widening = _Widening;

  public:
    class Accessor;

    template <
            typename PixelType, 
            int Width,
            int Height,
            int Depth,
            bool Compute_Variances = false
        >
    struct Fitter
    {
        typedef LeastSquaresLatticeFitter<
            Model,
            Deriver<Model,Width,Height>,
            PixelType,
            Width, Height, Depth,
            Compute_Variances> Type;
    };
};

}
}
#endif
