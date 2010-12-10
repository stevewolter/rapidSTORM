#define LIBFITPP_FITFUNCTIONS_CPP
#include "FitFunction.hh"

#include <cassert>
#include <limits>
#include <iostream>
#include <iomanip>

#include <Eigen/Cholesky> 
#include <Eigen/LU> 

using namespace std;
using namespace Eigen;

#define FITTING(x) 
//cerr << x << endl

namespace libfitpp {
    template <int varC>
    FitFunction<varC>::FitFunction() throw() {
        variables.setZero();
        startLambda = 1E-4;
        lambdaAdjustment = 16;
        maxSteps = 10;
        successiveNegligibleSteps = 1;
    }

    template <int varC>
    typename FitFunction<varC>::FitResult
    FitFunction<varC>::fit()
        throw() 
    {
        for (int i = 0; i < parameterIndex.size(); i++)
            if (isVariable(i))
                variableIndex[ parameterIndex[i] ] = i;

        double residues[2] = {-1,-1};
        Matrix<double, varC, 1> beta[2], diag[2], shift, savePosition,
                                bestPosition;
        Matrix<double, varC, varC> redHess[2];

        double bestResidues;

        int work = 1, trial = 0;

        double lambda = startLambda;
        int steps = maxSteps;

        int motivation = successiveNegligibleSteps, 
            curMotivation = motivation;

        bool tookValidStep = false, residuesGrew = true;

        /* This would be the right way to initialize, but is wasteful */
        if (!computeDerivatives(residues[work], beta[work], redHess[work]))
            return InvalidStartPosition;
        else {
            bestResidues = residues[work];
            bestPosition = variables;
            savePosition = variables;
            diag[work] = redHess[work].diagonal();
        }

        while (steps > 0 && curMotivation > 0) {
            /* Try to make a valid step */
            tookValidStep = false;
            do {
                /* Multiply the diagonal with the Marquardt factor */
                for (int j = 0; j < varC; j++)
                    redHess[work](j,j) = diag[work](j) * (1+lambda);

                bool haveStep = true;
                if (varC <= 4) {
                    shift = redHess[work].inverse() * beta[work];
                } else {
                    haveStep = redHess[work].ldlt().solve(beta[work], &shift);
                }

                if (haveStep) { 
                    if (!residuesGrew && changeIsNegligible(variables, shift) ) 
                    {
                        curMotivation--;
                        FITTING("Lambda " << lambda << ", accepting step length\n" << beta[work]);
                        FITTING("Accepting fit\n\n");
                    } else
                        curMotivation = motivation;

                    /* Take the suggested step. */
                    variables += shift;
                    steps--;

                    /* Compute the function at this place. */
                    tookValidStep = computeDerivatives
                        (residues[trial], beta[trial], redHess[trial]);
                }

                if (!tookValidStep) {
                    /* Revert the trial step and shorten the gradient
                     * step. */
                    variables = savePosition;
                    lambda *= lambdaAdjustment;
                }
            } while (steps > 0 && ! tookValidStep);

            FITTING("Took step from\n" << savePosition << "\n to \n" << variables);
            FITTING("Residues changed from " << residues[work] << " to "
                    << residues[trial]);
            /* The trial step was accepted. Check if it made matters
             * worse and save the savePosition. */
            if ( residues[trial] > residues[work] ) {
                lambda *= lambdaAdjustment;
                residuesGrew = true;
            } else {
                lambda /= lambdaAdjustment;
                residuesGrew = false;
                if (residues[trial] < bestResidues) {
                    bestPosition = variables;
                    bestResidues = residues[trial];
                }
            }

            savePosition = variables;
            /* These lines will switch trial and work. */
            trial = (1-trial);
            work = (1-work);

            /* Save diagonal for easier multiplication */
            diag[work] = redHess[work].diagonal();
        }

        variables = bestPosition;
        FITTING("Finished fitting\n");
        return FitSuccess;
    }


    template <int varC>
    void FitFunction<varC>::setVariableness(int parameter,
                bool isVariable) throw() 
    {
        if (parameterIndex.size() <= parameter)
            parameterIndex.resize( parameter+1 );
        
        parameterIndex[parameter] = (isVariable) ? 1 : -1;
        int nV = 0, nC = -1;
        for (int i = 0; i < parameterIndex.size(); i++)
            if ( parameterIndex[i] >= 0 ) {
                if (nV < varC)
                    variableIndex[nV] = i;
                parameterIndex[i] = nV;
                nV++;
            } else 
                parameterIndex[i] = nC--;

        if (constants.size() < -nC-1) 
            constants.resize(-nC-1);
    }

    template class FitFunction<1>;
    template class FitFunction<2>;
    template class FitFunction<3>;
    template class FitFunction<4>;
    template class FitFunction<5>;
    template class FitFunction<6>;
    template class FitFunction<7>;
}
