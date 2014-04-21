#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_EVALUATE_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_EVALUATE_H

#include "nonlinfit/plane/Distance.h"
#include <boost/bind/bind.hpp>

namespace nonlinfit {
namespace plane {

template <typename Number, int ChunkSize>
bool Distance<Number, ChunkSize>::evaluate(Derivatives& p)
{
    p.set_zero();

    for (auto& term : terms) {
        if (!term->StartIteration()) {
            return false;
        }
    }

    jacobian_.zero();
    for (auto& block : blocks) {
        Eigen::Matrix<Number, ChunkSize, 1> values =
            Eigen::Matrix<Number, ChunkSize, 1>::Zero();
        int offset = 0;
        for (auto& term : terms) {
            term->ComputeNextValuesAndDerivatives(&values, &jacobian_, &offset);
        }
        assert(offset == jacobian_.size());

        block.residues = block.output - values;
        switch (distance_metric) {
          case kSquaredDeviations:
            p.value += block.residues.square().sum();
            p.hessian.noalias() += jacobian_.transpose() * jacobian_;
            p.gradient.noalias() += jacobian_.transpose() * block.residues.matrix();
            break;
          case kPoissonMaximumLikelihood:
            p.value -= 2 * (block.residues - block.logoutput + values.log() * block.output).sum();
            auto quotient = block.output / values;
            p.hessian.noalias() += jacobian_.transpose() * (quotient / values).matrix().asDiagonal() * jacobian_;
            p.gradient.noalias() += jacobian_.transpose() * (quotient - 1).matrix();
            break;
        }

        assert( p.value == p.value );
    }
    
    return true;
}

}
}

#endif
