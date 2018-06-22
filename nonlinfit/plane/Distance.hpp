#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_EVALUATE_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_EVALUATE_H

#include "nonlinfit/VectorPosition.hpp"
#include "nonlinfit/plane/Distance.h"
#include <boost/bind/bind.hpp>

namespace nonlinfit {
namespace plane {

template <class Metric> struct increment_evaluation;

template <>
struct increment_evaluation<squared_deviations> {
    template <class Evaluation, class Values, class DataRow, class Jacobian>
    void operator()( 
        Evaluation& p, 
        const Values&, const DataRow& r, 
        const Jacobian& jac ) const
    {
        p.value += r.residues.square().sum();
        p.hessian.noalias() += jac.transpose() * jac;
        p.gradient.noalias() += jac.transpose() * r.residues.matrix();
    }
};

template <>
struct increment_evaluation<negative_poisson_likelihood> {
    template <class Evaluation, class Values, class DataRow, class Jacobian>
    void operator()( 
        Evaluation& p, 
        const Values& function, const DataRow& r,
        const Jacobian& jac ) const
    {
        p.value -= 2 * (r.residues - r.logoutput + function.log() * r.output).sum();
        typename DataRow::Output quotient = r.output / function;
        p.hessian.noalias() += jac.transpose() * (quotient / function).matrix().asDiagonal() * jac;
        p.gradient.noalias() += jac.transpose() * (quotient - 1).matrix();
    }
};

template <typename _Function, typename Tag, typename _Metric>
void Distance<_Function,Tag,_Metric>::evaluate_chunk( 
    Derivatives& p, const DataRow& r, const DataChunk& c )
{
    Eigen::Array<Number, Tag::ChunkSize, 1> values;

    this->evaluator.prepare_chunk( r.inputs );
    this->evaluator.value( values );
    c.residues = c.output - values;

    this->jac.compute( this->evaluator );

    increment_evaluation< _Metric >()( p, values, c, *this->jac );
    assert( p.value == p.value );
}


template <typename _Function, typename Tag, typename _Metric>
bool Distance<_Function,Tag,_Metric>::evaluate(Derivatives& p)
{
    p.set_zero();

    if ( ! evaluator.prepare_iteration( *xs ) )
        return false;
    
    jac.precompute( evaluator );
    assert(this->xs->data.size() == this->ys->size());
    auto j = this->ys->begin();
    for (auto i = this->xs->data.begin(); i != this->xs->data.end(); ++i, ++j) {
        evaluate_chunk(p, *i, *j);
    }

    return true;
}

template <typename _Function, typename Num, int _ChunkSize, typename P1, typename P2>
void Distance< _Function, Disjoint<Num,_ChunkSize,P1,P2>, squared_deviations >
::evaluate_chunk( Derivatives& p, const OuterJacobian& dx, const DataRow& r, const DataChunk& c )
{
    Eigen::Array<Number, _ChunkSize, 1> values;

    evaluator.prepare_chunk( r.inputs );
    evaluator.value( values );
    c.residues = c.output - values;
    p.value += c.residues.square().sum();

    /* Compute the Y parts of the derivatives by part. */
    nonlinfit::Jacobian<Num,1,InnerTerms> dy;
    dy.compute( evaluator );

    gradient_accum +=
        ((dx->transpose() * c.residues.matrix()).array()
            * dy->transpose().array()).matrix();
    y_hessian += dy->transpose() * *dy;

    assert( p.value == p.value );
}
template <typename _Function, typename Num, int _ChunkSize, typename P1, typename P2>
bool Distance< _Function, Disjoint<Num,_ChunkSize,P1,P2>, squared_deviations >
::evaluate( Derivatives& p) 
{
    p.value = 0;
    gradient_accum.fill(0);
    y_hessian.fill(0);

    if ( ! evaluator.prepare_iteration( *xs ) )
        return false;
    
    /* Pre-compute the outer jacobian here so it will be available to compute
     * the residues in the inner loop. */
    OuterJacobian dx;
    dx.compute( evaluator );

    assert(this->xs->data.size() == this->ys->size());
    auto j = this->ys->begin();
    for (auto i = this->xs->data.begin(); i != this->xs->data.end(); ++i, ++j) {
        evaluate_chunk(p, dx, *i, *j);
    }

    /* Compute the hessian matrix of derivation summands by multiplying
     * X and Y contributions. */
    Eigen::Matrix<Num,TermCount,TermCount> x_hessian = dx->transpose() * *dx;

    combiner.matrix(p.hessian, (x_hessian.array() * y_hessian.array()).matrix());
    combiner.vector(p.gradient, gradient_accum);

    return true;
}

}
}

#endif
