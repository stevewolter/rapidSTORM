#ifndef NONLINFIT_EVALUATION_HPP
#define NONLINFIT_EVALUATION_HPP

#include <stdexcept>
#include "nonlinfit/Evaluation.h"

namespace nonlinfit {

template <class Num, int ChunkSize, int MaxChunkSize>
bool Evaluation<Num,ChunkSize,MaxChunkSize>::operator==( const Evaluation& o ) const
{
    Num epsilon = 1E-2;

    return ( ( (gradient - o.gradient).array().abs() / (gradient + o.gradient).array().abs()).maxCoeff() < epsilon ) &&
           ( ((hessian - o.hessian).array().abs() / (hessian + o.hessian).array().abs()).maxCoeff() < epsilon ) &&
           ( std::abs( value - o.value ) / std::max( std::abs(value), std::abs(o.value) ) < epsilon );
}

template <class Num, int VarCount, int MaxVarCount>
std::ostream& operator<<( std::ostream& o, const Evaluation<Num,VarCount,MaxVarCount>& e )
{
    Eigen::IOFormat f(12);
    return o << e.value << " : " << e.gradient.transpose().format(f) << "\n"
             << e.hessian.format(f);
}

}

#endif
