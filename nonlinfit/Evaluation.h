#ifndef NONLINFIT_DERIVATIVES_H
#define NONLINFIT_DERIVATIVES_H

#include "nonlinfit/fwd.h"
#include <Eigen/Core>
#include <iosfwd>
#include <boost/mpl/size.hpp>

namespace nonlinfit {

/** \brief Value and partial derivatives of a Function. */
template <class Num>
struct Evaluation
{ 
    typedef Num Scalar;

    /** An Eigen vector with one row per function variable. */
    typedef Eigen::Matrix<Num, Eigen::Dynamic, 1, Eigen::ColMajor | Eigen::AutoAlign> Vector;
    /** A square Eigen matrix with one row and column per function variable. */
    typedef Eigen::Matrix<
        Num, Eigen::Dynamic, Eigen::Dynamic,
        Eigen::ColMajor | Eigen::AutoAlign> Matrix;

    /** The vector \f$ -0.5 \frac{ \partial f }{ \partial p_i } \f$. 
     *  In words: Each element of this vector gives the partial derivative of #value
     *  with respect to the parameter at the given index. */
    Vector gradient;
    /** A matrix approximating to \f$ 0.5 \frac{ \partial^2 f }{ \partial p_i 
     *  \partial p_j } \f$. In words: Each element of this matrix gives
     *  the partial second derivative of #value with respect to the two
     *  parameters defined by its indices. Implementing classes
     *  are free to use the Levenberg-Marquardt approximation here, i.e.
     *  approximating the Hessian by the outer product of the gradient, 
     *  summed over all points. */
    Matrix hessian;
    /** The result of a function evaluation. */
    Num value;

    /** Constructor. 
     *  \param varc For dynamically sized functions, the number of parameters.
     */
    Evaluation( int varc )
        : gradient(varc,1), hessian(varc,varc) 
        { assert( varc == Vars || Vars == Eigen::Dynamic ); }
    /** Set all members fields to zero values. Useful for incremential 
     *  computation of the fields. */
    void set_zero() { gradient.fill(0); hessian.fill(0); value = 0; }
    /** Check if any element is NaN.
     *  \return true iff any of the field values is not a number (NaN). */
    bool contains_NaN() const { 
        return ! ( value == value ) ||
               ! ( gradient == gradient ) ||
               ! ( hessian == hessian );
    }

    /** Compare to evaluations up to a relative epsilon. */
    bool operator==( const Evaluation& other ) const;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

template <typename Num>
std::ostream& operator<<( std::ostream&, const Evaluation<Num,VarCount,MaxVarCount>& );

}

#endif
