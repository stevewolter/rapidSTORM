#ifndef LIBFITPP_FITFUNCTIONS_H
#define LIBFITPP_FITFUNCTIONS_H

#include <iostream>
#include <Eigen/Core>
#include <stdexcept>
#include <fit++/Position.hh>
#include <fit++/Helpers.hh>

namespace fitpp {

enum FitResult {
    FitSuccess,
    InvalidStartPosition,
    FoundNoInitialStep,
    SingularMatrix
};

template <int VarC>
class Derivatives {
  private:
    Eigen::Matrix<double,VarC,1> orig_alpha_diag;

  public:
    Eigen::Matrix<double,VarC,VarC> alpha;
    Eigen::Matrix<double,VarC,1> beta;

    void save_diag() { orig_alpha_diag = alpha.diagonal(); }
    void apply_lambda(double lambda) 
        { alpha.diagonal() = orig_alpha_diag * (1 + lambda); }
};

/** The FitFunction class provides parameter-based fitting. */
template <int VarC, bool Compute_Variances = false> 
class FitFunction {
  public:
    typedef Eigen::Matrix<double, VarC, 1> ParamVector;
    typedef Eigen::Matrix<double, VarC, VarC> ParamMatrix;
    
    FitFunction();
    /** Destructor. */
    virtual ~FitFunction() {}

  public:
    template <typename DeriverType>
    std::pair<FitResult,typename DeriverType::Position*>
    fit(
        typename DeriverType::Position& starting_position,
        typename DeriverType::Position& workspace,
        const typename DeriverType::Constants& constants,
        const DeriverType& deriver
    ) const;

    inline void setStartLambda(double value) 
        { startLambda = value; }
    inline void setLambdaAdjustment(double value)
        { lambdaAdjustment = value; }
    inline void setMaximumIterationSteps(int stepNum)
        { maxSteps = stepNum; }
    inline void setSuccessiveNegligibleStepLimit(int number)
        { successiveNegligibleSteps = number; }

    void set_absolute_epsilon(int variable, double eps);

  protected:
    double startLambda, lambdaAdjustment;
    int maxSteps, successiveNegligibleSteps;
    ParamVector abs_eps, rel_eps;
    bool abs_eps_set, rel_eps_set;

    bool change_is_negligible( 
        const ParamVector& position, 
        const ParamVector& shift ) const
    {
        bool abs_is_off = abs_eps_set && 
            (shift.cwise().abs().cwise() >= abs_eps).any();
        bool rel_is_off = rel_eps_set && 
            ((shift.cwise()/position).cwise().abs()
                .cwise() >= abs_eps).any();
        return ! ( abs_is_off || rel_is_off );
    };

  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

#if 0
template <int VarC, bool Save_Residues, bool Compute_Variances> 
class FitFunction<VarC, Save>::_Position
    : public virtual fitpp::Position<VarC>
{
  protected:
    friend class FitFunction<VarC, Flags>;
    void save_derivatives( const Derivatives<VarC>& d ) throw() {
        this->covariances = d.alpha;
        this->residues = d.residues;
        this->chi_sq = d.chi_sq;
    }

    void compute_covariances() throw() {
        this->covariances = 
            this->covariances.eval().inverse();
    }

  protected:
    _Position() throw() {}
  public:
    _Position( 
        const typename fitpp::Position<VarC>::Parameterness& p 
    ) throw()
        : fitpp::Position<VarC>(p) {}

    const double& get_parameter_variance(int index) const throw() {
        assert( index < this->variable_to_cp.size() );
        if ( (Flags & Compute_Parameter_Variances) == 0 )
            throw std::logic_error("Fitter not instantiated for variances.");

        const int& i = this->variable_to_cp[index];
        if ( (i & this->type_part) == 0 ) {
            return this->covariances.diagonal()[i & this->index_part];
        } else
            return 0;
    }
};
#endif

}

#endif