#ifndef LIBFITPP_EXPONENTIAL2D_H
#define LIBFITPP_EXPONENTIAL2D_H

#include <fit++/FitFunction.hh>
#include <fit++/LatticeFitFunction.hh>
#include <fit++/BitfieldConstructor.hh>
#include <vector>

#include <iostream>
using namespace std;

namespace fitpp {

/** The Exponential2D class fits the given lattice points with an
    *  exponential model of the form 
    *  A * exp( -1/2 * (x-x0)^2 / sigma_x^2 * (y-y0)^2 / sigma_y^2 ) + B.
    *
    *  All of these parameters must be declared as fixed or fitted
    *  at construction time. */
namespace Exponential2D {

enum Names { 
    Shift = 0,  /**< B. Only one for all functions */
    MeanX = 1, /**< x0 */
    MeanY = 2, /**< y0 */
    Amplitude = 3, /**< A */
    Amp = Amplitude,
    SigmaX = 4, /**< sigma_x */ 
    SigmaY = 5, /**< sigma_y */ 
    SigmaXY = 6, /**< Correlation between axes */
    NumNames = 7,
    FunctionDeps = 6,
    Globals = 1
};

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
struct For {

static const int GlobalMask = (0x1 << Globals) - 1;

static const int ParamC = Globals + FunctionDeps * Kernels;
static const int GlobalVarC = Bits<ParameterMask & GlobalMask>::Count;
static const int GlobalConstC = Globals - GlobalVarC;
static const int LocalVarC = Bits<ParameterMask & (~GlobalMask)>::Count;
static const int VarC = GlobalVarC + Kernels * LocalVarC;
static const int ConstC = ParamC - VarC;

typedef OptionalMatrix<double,ConstC,1> Constants; 
//typedef fitpp::LatticePosition<VarC> Position;
typedef typename fitpp::Position<VarC>::Vector Variables;

private:
struct VariableInfo { bool variableness; int offset, step; };

public:
template <int Name>
class ParamTraits {
    static const int ConstOffset = (Globals - GlobalVarC),
                    ConstStep = (FunctionDeps - LocalVarC);
    static const int VarOffset = GlobalVarC, VarStep = LocalVarC;
  public:
    static const bool Global = (Name < Globals), 
                      Variable = (ParameterMask & (1 << Name));

    static const int ParamOffset = Name;
    static const int ParamStep = (Global) ? 0 : FunctionDeps;

  private:
    static const int Vars_before_this 
        = Bits<ParameterMask & ((1 << Name)-1)>::Count;
    static const int Elements_before_this
        = (Variable) ?  Vars_before_this : Name - Vars_before_this;
    static const int Global_elements_before_this
        = (Global) ? Name : ((Variable) ? GlobalVarC : GlobalConstC);
    static const int Local_Elements_before_this
        = Elements_before_this - Global_elements_before_this;

    static const int Offset = Global_elements_before_this;
    static const int Step = (Global) ? 0 : 1;
    static const int Stride = (Global) ? 0 : Local_Elements_before_this;

  public:
#if 0
    typedef std::vector< VariableInfo> argument_type;
    static void for_loop_call(argument_type& i) throw() { 
        i[Name].variableness = Variable; 
        i[Name].offset = Offset;
        i[Name].step = Step;
    }
#endif

    template <int Kernel>
    struct Index {
        static const int N = Stride * Kernels + Step * Kernel + Offset;
    };

};

template <int Name, int Kernel>
struct ParamIndex {
    static const int N = ParamTraits<Name>::template Index<Kernel>::N;

    static double& value(Variables &v, Constants& c) throw() {
        if ( ParamTraits<Name>::Variable ) return v[N]; else return c[N];
    }
    static double value(const Variables &v, const Constants& c) 
        throw() 
    {
        if ( ParamTraits<Name>::Variable ) return v[N]; else return c[N];
    }
};

public:

typedef typename Eigen::Matrix<double,VarC,VarC> Matrix;
typedef typename Eigen::Matrix<double,VarC,1> Vector;

template <Names name, int Kernel, bool ComputeVariance>
static void set_absolute_epsilon( 
    FitFunction< VarC,ComputeVariance>& fit_function, double value 
) {
    fit_function.set_absolute_epsilon
        ( ParamIndex<name,Kernel>::N, value );
    if ( Kernel > 0 )
        set_absolute_epsilon<name,
                             (Kernel>0) ? Kernel - 1 : 0,
                             ComputeVariance>
            (fit_function, value);
}

class NamedParameters {
    Variables *variables;
    Constants *constants;
  public:
    NamedParameters(Variables* variables, Constants *constants) 
        : variables(variables), constants(constants) {}

    void change_variable_set(Variables* variables) 
        { this->variables = (variables); }

    template <Names name, int function>
    double get() const 
        {return ParamIndex<name,function>::value(*variables, *constants);}

    template <Names name, int function>
    void set(double value) 
        {ParamIndex<name,function>::value(*variables, *constants) = value;}

    template <int Function>
    double getSigmaX() const { return get<SigmaX,Function>(); }
    template <int Function>
    double getSigmaY() const { return get<SigmaY,Function>(); }
    template <int Function>
    double getSigmaXY() const { return get<SigmaXY,Function>(); }
    template <int Function>
    double getMeanX() const { return get<MeanX,Function>(); }
    template <int Function>
    double getMeanY() const { return get<MeanY,Function>(); }
    template <int Function>
    Eigen::Vector2d getPosition() const {
        return Eigen::Vector2d( 
            get<MeanX,Function>(), get<MeanY,Function>() );
    }
    template <int Function>
    double getAmplitude() const { return get<Amp,Function>(); }

    template <int Function>
    void setSigmaX(double v) { return set<SigmaX,Function>(v); }
    template <int Function>
    void setSigmaY(double v) { return set<SigmaY,Function>(v); }
    template <int Function>
    void setSigmaXY(double v) { return set<SigmaXY,Function>(v); }
    template <int Function>
    void setMeanX(double v) { return set<MeanX,Function>(v); }
    template <int Function>
    void setMeanY(double v) { return set<MeanY,Function>(v); }
    template <int Function>
    void setAmplitude(double v) { return set<Amp,Function>(v); }

    double getShift() const { return get<Shift,0>(); }
    void setShift(double v) { return set<Shift,0>(v); }
};

template <typename DataType,
          int Width, int Height,
          bool Corr = true>
class Deriver
: public LatticeFunction<DataType,Width,Height>
{
  public:
    typedef typename Exponential2D::Deriver
        <Kernels,ParameterMask,Width,Height,Corr>
        RealDeriver;
    typedef LatticePosition<VarC,Width,Height>
        Position;

    RealDeriver rd;

    typedef typename For<Kernels,ParameterMask>::Constants Constants;

    static const bool Dynamic = 
        (Width == Eigen::Dynamic || Height== Eigen::Dynamic);

    bool compute_derivatives( 
        Position& pos,
        Derivatives<VarC>& derivatives,
        const Constants& con
    ) const {
        bool ok = LatticeFunction<DataType,Width,Height>::
          compute_derivatives
            ( pos, derivatives, con, 
              const_cast<RealDeriver&>(rd) );
        if (!ok) return false;

        return true;
    }

    void setSize(int width, int height) {
        LatticeFunction<DataType,Width,Height>::
            setSize(width,height);
        rd.resize( width, height );
    }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

template <typename DataType, 
          int Width = Eigen::Dynamic, int Height = Eigen::Dynamic,
          bool Use_Correlation = ParamTraits<SigmaXY>::Variable,
          bool Compute_Variances = false>
struct FitObject
: public NamedParameters,
  public Deriver<DataType,Width,Height,Use_Correlation>,
  public FitFunction<VarC, Compute_Variances>
{
  public:
    typedef typename Deriver<DataType,Width,Height,Use_Correlation>::Position Position;
  private:
    std::vector<VariableInfo> i;
#if 0
    Names meanings[VarC];
#endif

    Position a, b, *current;
    Constants constants;
  public:

    FitObject() 
    : NamedParameters( &a.parameters, &constants ),
      i(NumNames),
      current( &a )
    {
#if 0
        ForLoop<NumNames-1,ParamTraits>::execute(i);
        for (int j = 0; j < NumNames; j++)
            if ( i[j].variableness )
              for (int v = i[j].offset; v < VarC; 
                     v += (i[j].step) ? i[j].step : VarC)
                meanings[v] = (Names)j;
#endif
    }

    FitResult fit() { 
        std::pair<FitResult,Position*> result = 
            this->FitFunction<VarC, Compute_Variances>::
                fit( *current, (&a==current)?b:a, constants, *this );

        if ( result.second != NULL ) {
            current = result.second;
            change_variable_set( &current->parameters );
        }
        return result.first;
    }

#if 0
    Names get_parameter_meaning(int v) const throw()
        { return meanings[v]; }
#endif

    template <Names name>
    bool isVariable() const 
        { return ParamTraits<name>::Variable; }
    bool is_variable(Names name) const 
        { return i[name].variableness; }

    template <Names name>
    void set_absolute_epsilon( double value ) {
        For<Kernels,ParameterMask>::template 
        set_absolute_epsilon<name,Kernels-1,Compute_Variances>(
            static_cast<FitFunction<VarC, Compute_Variances>&>(*this),
            value );
    }

    const ResidueMatrix<Height,Width>&
        get_residues() const 
        { return this->residues; }

    void setSize(int width, int height) {
        a.resize(width,height);
        b.resize(width,height);
        Deriver<DataType,Width,Height,Use_Correlation>
            ::setSize(width,height);
    }

    const Position& getPosition() { return *current; }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

};

}
}
#endif
