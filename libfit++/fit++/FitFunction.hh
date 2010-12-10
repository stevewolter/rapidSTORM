#ifndef LIBFITPP_FITFUNCTIONS_H
#define LIBFITPP_FITFUNCTIONS_H

#include <iostream>
#include <Eigen/Core>

namespace libfitpp {

    /** The Fitter class is the base class for classes
     *  that support parameter fitting to data. It offers the fit()
     *  method as a public interface for starting the fitting
     *  process. */
    class Fitter {
      public:
        virtual ~Fitter() throw() {}

        enum FitResult { FitSuccess, InvalidStartPosition, FitFailure };
        /** Fit the supplied data with the function defined
         *  in the implementing class. When the residues shrink,
         *  but not more than min_residues_change * last_residue,
         *  abort.
         *
         *  \param min_residues_change When residues shrink less
         *                             than this value times the
         *                             last residues, abort. */
        virtual FitResult fit() throw() = 0;
    };

    /** The FitFunction class provides parameter-based fitting. */
    template <int varC>
    class FitFunction : public Fitter {
      public:
        FitFunction() throw();
        /** Destructor. */
        virtual ~FitFunction() throw() {}

        virtual FitResult fit() throw();

        /** Set parameter number \c parameter to value \c value.
         *  See implementing class for valid parameter values. */
        inline void setParameter(int parameter, const double value)
            throw() 
        {
            int index = parameterIndex[parameter];
            if (index >= 0)     variables[index] = value;
            else                constants[-index-1] = value;
        }
        /** Get value of parameter number \c parameter.
         *  See implementing class for valid parameter values. */
        inline double getParameter(int parameter) const throw() {
            int index = parameterIndex[parameter];
            if (index >= 0)     return variables[index];
            else                return constants[-index-1];
        }

        /** Returns whether the parameter number \c parameter
         *  is a variable (that is, fitted to the data) or a 
         *  constant. */
        inline bool isVariable(int parameter) const throw() 
          { return parameterIndex[parameter] >= 0; }
        void setVariableness(int parameter, bool isVariable) throw();
        bool getVariableness(int parameter) const throw() 
            { return isVariable(parameter); }

        inline void setStartLambda(double value) throw() 
            { startLambda = value; }
        inline void setLambdaAdjustment(double value) throw()
            { lambdaAdjustment = value; }
        inline void setMaximumIterationSteps(int stepNum) throw()
            { maxSteps = stepNum; }
        inline void setSuccessiveNegligibleStepLimit(int number) throw()
            { successiveNegligibleSteps = number; }

      protected:
        typedef Eigen::Matrix<double, varC, 1> Gradient;
        typedef Eigen::Matrix<double, varC, varC> Curvature;

        Eigen::Matrix<double, varC, 1> variables;
        Eigen::Matrix<int, varC, 1> variableIndex;
        Eigen::VectorXi parameterIndex;
        Eigen::VectorXd constants;

        virtual bool changeIsNegligible(const Gradient &position,
                                        const Gradient &step) throw() = 0;

        double startLambda, lambdaAdjustment;
        int maxSteps, successiveNegligibleSteps;

      protected:
        /** Compute the gradient vector and the reduced Hessian matrix
         *  for the parameters of the model function which are true
         *  in the variable array. The current position is given
         *  in parameters. */
        virtual bool computeDerivatives(
            double &residuesSq, Gradient& gradient, Curvature& redHess)
            throw() = 0;

      public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };
}

#endif
