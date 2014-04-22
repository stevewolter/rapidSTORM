#ifndef NONLINFIT_TERM_H
#define NONLINFIT_TERM_H

#include <Eigen/Core>

namespace nonlinfit {

template <typename Number, int ChunkSize>
class Term {
  public:
    virtual ~Term() {}

    virtual bool StartIteration() = 0;
    virtual void ComputeNextValuesAndDerivatives(
        Eigen::Matrix<Number, ChunkSize, 1>* values,
        Eigen::Matrix<Number, ChunkSize, Eigen::Dynamic>* jacobian,
        int* offset) = 0;
};

}

#endif
