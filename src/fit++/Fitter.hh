#ifndef LIBFITP_FITTER_HH
#define LIBFITP_FITTER_HH

#include "LatticeFunction.hh"
#include "LatticeFunction.hh"

namespace fitpp {

template <typename NamedParameters,
          typename Deriver,
          typename DataType, 
          int Width = Eigen::Dynamic, int Height = Eigen::Dynamic,
          bool Compute_Variances = false>
struct LeastSquaresLatticeFitter
: public NamedParameters,
  public LatticeFunction<DataType,Width,Height>,
  public Deriver {
  public:
    typedef LatticePosition<VarC,Width,Height> Position;
    typedef typename ParameterMap::Constants Constants;

  private:
    Position a, b, *current;
    Constants constants;
    FitFunction<VarC, Compute_Variances> lsq_fitter;
  public:

    Fitter() 
    : NamedParameters( &a.parameters, &constants ),
      current( &a ) { }

    FitResult fit() { 
        std::pair<FitResult,Position*> result = 
            lsq_fitter.template fit_with_deriver<FitObject>
                ( *current, (&a==current)?b:a, constants, *this );

        if ( result.second != NULL ) {
            current = result.second;
            change_variable_set( &current->parameters );
        }
        return result.first;
    }

    bool compute_derivatives( 
        Position& pos,
        Derivatives<VarC>& derivatives,
        const Constants& con
    ) {
        bool ok = LatticeFunction<DataType,Width,Height>::
          compute_derivatives
            ( pos, derivatives, con, *this );
        if (!ok) return false;

        return true;
    }

    template <Names name>
    void set_absolute_epsilon( double value ) {
        For<Kernels,ParameterMask>::template 
        set_absolute_epsilon<name,Kernels-1,Compute_Variances>(
            lsq_fitter, value );
    }

    const ResidueMatrix<Height,Width>&
        get_residues() const 
        { return this->residues; }

    void setSize(int width, int height) {
        a.resize(width,height);
        b.resize(width,height);
        LatticeFunction<DataType,Width,Height>::
            setSize(width,height);
        Deriver::resize( width, height );
    }

    const Position& getPosition() { return *current; }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}

#endif
