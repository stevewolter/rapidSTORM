#ifndef LIBFITP_FITTER_HH
#define LIBFITP_FITTER_HH

namespace fitpp {

template <typename Model,
          typename Deriver,
          typename DataType, 
          int Width = Eigen::Dynamic, int Height = Eigen::Dynamic,
          bool Compute_Variances = false>
struct LeastSquaresLatticeFitter
: public LatticeFunction<DataType,Width,Height>,
  public Deriver 
{
  public:
    typedef LatticePosition<Model::VarC,Width,Height> Position;
    typedef typename Model::Variables Variables;
    typedef typename Model::Constants Constants;
    typedef FitFunction<Model::VarC, Compute_Variances> Function;

  private:
    Position position;
    Constants constants;

  public:
    FitResult fit( Function& function ) {
        Position second;
        std::pair<FitResult,Position*> result = 
            function.fit_with_deriver( position, second, *this );

        if ( result.second == &second )
            position = second;
        return result.first;
    }

    bool compute_derivatives( 
        Position& pos,
        Derivatives<Model::VarC>& derivatives
    ) {
        bool ok = LatticeFunction<DataType,Width,Height>::
          compute_derivatives
            ( pos, derivatives, constants, *this );
        if (!ok) return false;

        return true;
    }

    const ResidueMatrix<Height,Width>&
        get_residues() const 
        { return this->residues; }

    void setSize(int width, int height) {
        LatticeFunction<DataType,Width,Height>::
            setSize(width,height);
        Deriver::resize( width, height );
    }

    const Position& getPosition() const { return position; }
    Position& getPosition() { return position; }
    const Variables& getVariables() const { return position.parameters; }
    Variables& getVariables() { return position.parameters; }
    const Constants& getConstants() const { return constants; }
    Constants& getConstants() { return constants; }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}

#endif
