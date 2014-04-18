#ifndef NONLINFIT_LEVENBERGMARQUARDT_EXCEPTIONS_H
#define NONLINFIT_LEVENBERGMARQUARDT_EXCEPTIONS_H

#include "nonlinfit/levmar/fwd.h"
#include <stdexcept>
#include <Eigen/Core>

namespace nonlinfit {
namespace levmar {

/** This exception is thrown when evaluating the function at the start
 *  position returns false. */
struct InvalidStartPosition
: public std::runtime_error { 
    /** The failing start position. */
    Eigen::MatrixXd offender;
    template <typename Type>
    InvalidStartPosition( const Eigen::MatrixBase<Type>& offending_matrix )
        : runtime_error("Invalid start position"), offender(offending_matrix) {}
    ~InvalidStartPosition() throw() {}
};
/** This exception is thrown when the LM equation system cannot be solved. */
struct SingularMatrix
: public std::runtime_error { 
    Eigen::MatrixXd offender;
    template <typename Type>
    SingularMatrix( const Eigen::MatrixBase<Type>& offending_matrix ) 
        : runtime_error("Hessian matrix at fit position got singular"), offender(offending_matrix) {}
    ~SingularMatrix() throw() {}
};

}
}

#endif
