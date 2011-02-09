#ifndef LIBFITP_FITTER_HH
#define LIBFITP_FITTER_HH

namespace fitpp {

template <typename Model,
          typename Deriver,
          typename DataType, 
          int Width, int Height, int Depth,
          bool Compute_Variances>
struct LeastSquaresLatticeFitter
: public LatticeFunction<DataType,Width,Height,Depth>,
  public Deriver 
{
  public:
    typedef LatticePosition<Model::VarC,Width,Height,Depth> Position;
    typedef typename Model::Variables Variables;
    typedef typename Model::Constants Constants;
    typedef FitFunction<Model::VarC, Compute_Variances> Function;

  private:
    Position position;
    const Constants& constants;

  public:
    LeastSquaresLatticeFitter(const Constants& constants) 
        : constants(constants) {}

    FitResult fit( Function& function ) {
        Position second;
        second.resize( position );
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
        bool ok = LatticeFunction<DataType,Width,Height,Depth>::
          compute_derivatives
            ( pos, derivatives, constants, *this );
        if (!ok) return false;

        return true;
    }

    const ResidueMatrix<Height,Width>&
        get_residues() const 
        { return this->residues; }

    void setSize(int width, int height) {
        LatticeFunction<DataType,Width,Height,Depth>::
            setSize(width,height);
        Deriver::resize( width, height );
        position.resize(width,height);
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
