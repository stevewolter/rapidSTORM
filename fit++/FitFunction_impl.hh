#ifndef LIBFITPP_FITFUNCTIONS_CPP
#define LIBFITPP_FITFUNCTIONS_CPP

#include "debug.h"
#include "FitFunction.hh"

#include <cassert>
#include <limits>
#include <iostream>
#include <iomanip>
#include <cmath>

#include <Eigen/Cholesky> 
#include <Eigen/LU> 
#include <Eigen/Array>

namespace fitpp {
using namespace std;
using namespace Eigen;

static const double infty = numeric_limits<double>::infinity();

template <int VarC, bool SR>
FitFunction<VarC,SR>::FitFunction() 
: abs_eps(ParamVector::Constant( infty ) ),
  rel_eps(abs_eps),
  abs_eps_set(false), rel_eps_set(false)
{
    startLambda = 1E-4;
    lambdaAdjustment = 10;
    maxSteps = 30;
    successiveNegligibleSteps = 2;
}

template <int VarC, bool CV>
template <typename DeriverType>
std::pair<FitResult,typename DeriverType::Position*>
FitFunction<VarC,CV>::fit_with_deriver(
    typename DeriverType::Position& max,
    typename DeriverType::Position& moritz,
    DeriverType& deriver
) const
{
    Derivatives<VarC> der[ 2 ];

    double lambda = startLambda;
    ParamVector shift;
    ParamMatrix inverted;

    typename DeriverType::Position 
        *work_p = &max, *trial_p = &moritz;

    Derivatives<VarC> *work_d = der+0, 
                      *trial_d = der+1;

    int motivation = successiveNegligibleSteps, 
        curMotivation = motivation;

    DEBUG("Beginning fitting, motivation is " << motivation);
    bool residuesGrew = true;
    bool tookOneValidStep = false;

    DEBUG("Supplied start " << work_p->parameters.transpose());

    const bool initial_position_valid =
        deriver.compute_derivatives
                        ( *work_p, *work_d );
    if ( ! initial_position_valid )
        return make_pair(InvalidStartPosition, (typename DeriverType::Position*)NULL);

    work_d->save_diag();

    DEBUG("Starting residues are " << work_p->chi_sq << ", going to fit "
            " for " << maxSteps << " steps.");

    for (int steps = maxSteps; steps && curMotivation > 0; ) {
        /* Multiply the diagonal with the Marquardt factor */
        work_d->apply_lambda( lambda );

        /* Solve the equation system for the shift vector.*/
        if (VarC <= 4) {
            DEBUG("Position " << work_p->parameters.transpose());
            #ifndef NDEBUG
            if ( abs( work_d->alpha.determinant() ) < 1E-50 ) {
                std::cerr << "Determinant of covariance matrix is very small:"
                          << work_d->alpha.determinant() << "\n"
                          << work_d->alpha << "\n";
                std::cerr << "Computed for position "
                          << work_p->parameters.transpose() << "\n";
                lambda *= 10;
                continue;
            }
            #endif
            work_d->alpha.computeInverse(&inverted);
            shift = inverted * work_d->beta;
            DEBUG("Resulting shift is " << shift.transpose());
            assert( !isnan( shift[0] ) &&
                    !isnan( shift[1] ) &&
                    !isnan( shift[2] ) &&
                    !isnan( shift[3] ) );
        } else {
            DEBUG("Solving equations for " << work_d->alpha << "\n" << work_d->beta.transpose());
            bool solvable = work_d->alpha.ldlt().solve(work_d->beta, &shift);
            if (!solvable)  {
                DEBUG("Equation system unsolvable, raising lambda");
                lambda *= 100;
                continue;
            } else {
                DEBUG("Solved equation system for step " << shift.transpose());
            }
        }

        if (!residuesGrew && 
            change_is_negligible(work_p->parameters, shift) ) 
        {
            curMotivation--;
            if ( curMotivation == 0 ) {
                DEBUG("Lambda " << lambda << ", "
                        "accepting step length\n" << shift.transpose());
            } else {
                DEBUG("Negligible step " << shift.transpose() << 
                        " results in motivation " 
                        << curMotivation);
            }
        } else
            curMotivation = motivation;

        /* Take the suggested step. */
        trial_p->parameters = work_p->parameters + shift;
        steps--;

        /* Compute the function at this place. */
        bool new_position_valid = 
            deriver.compute_derivatives( *trial_p, *trial_d);

        if ( new_position_valid ) {
            DEBUG("New position is valid, residues are " << trial_p->residues);
        }

        /* The trial step was evaluable. Check if it improved
            * residues. */
        if ( !new_position_valid ||
             trial_p->chi_sq > work_p->chi_sq ) 
        {
            lambda *= lambdaAdjustment;
            DEBUG("Residues worsened from "
                    << work_p->chi_sq << " to " 
                    << trial_p->chi_sq << ". "
                    << "Changed lambda to " << lambda);
            residuesGrew = true;
        } else {
            lambda /= lambdaAdjustment;
            residuesGrew = false;

            DEBUG("Accepted step from\n" << work_p->parameters.transpose() << "\n to \n" << trial_p->parameters.transpose());
            DEBUG("Residues changed from " << work_p->chi_sq << " to "
                    << trial_p->chi_sq);

            tookOneValidStep = true;
            std::swap( work_p, trial_p );
            std::swap( work_d, trial_d );
            work_d->save_diag();
        }
        DEBUG("Have " << steps << " steps left\n");
    }

    if ( CV ) {
        work_d->apply_lambda(0);
        work_d->alpha.computeInverse(&work_p->covariances);
    }
    DEBUG("Finished fitting\n");
    return make_pair(
        (tookOneValidStep) ? 
            FitSuccess : FoundNoInitialStep,
        work_p);
}

template <int VarC, bool CV>
void FitFunction<VarC,CV>::set_absolute_epsilon
    (int variable, double eps)
{
    abs_eps[variable] = eps;
    abs_eps_set = ( abs_eps.cwise() != ParamVector::Constant( infty ) ).any();
}

}

#endif
