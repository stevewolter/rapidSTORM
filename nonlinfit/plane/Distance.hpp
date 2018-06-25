#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_EVALUATE_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_EVALUATE_H

#include "Distance.h"
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
        if (Values::RowsAtCompileTime == 1 && Values::ColsAtCompileTime == 1) {
            // Special case for Eigen bug: Diagonal of 1x1 times 1xN matrix breaks for odd N.
            p.hessian.noalias() += (jac.transpose() * jac) * (r.output(0,0) / (function(0,0)*function(0,0)));
        } else {
            Jacobian divided_jac = function.inverse().matrix().asDiagonal() * jac;
            p.hessian.noalias() += divided_jac.transpose() * r.output.matrix().asDiagonal() * divided_jac;
        }
        p.gradient.noalias() += jac.transpose() * (quotient - 1).matrix();
    }
};

template <typename _Function, typename Tag, typename _Metric>
void Distance<_Function,Tag,_Metric>::operator()( 
    Derivatives& p, const DataRow& r )
{
    typename DataRow::Output values;

    this->evaluator.prepare_chunk( r.inputs );
    this->evaluator.value( values );
    r.residues = r.output - values;

    this->jac.compute( this->evaluator );

    increment_evaluation< _Metric >()( p, values, r, *this->jac );
    assert( p.value == p.value );
}


template <typename _Function, typename Tag, typename _Metric>
bool Distance<_Function,Tag,_Metric>::evaluate(Derivatives& p)
{
    p.set_zero();

    if ( ! evaluator.prepare_iteration( *data ) )
        return false;
    
    jac.precompute( evaluator );
    std::for_each( data->chunk_view().begin(), data->chunk_view().end(), 
        boost::bind( boost::ref(*this), boost::ref(p), boost::arg<1>() ) );

    return true;
}

template <typename _Function, typename Num, int _ChunkSize, typename P1, typename P2>
void Distance< _Function, Disjoint<Num,_ChunkSize,P1,P2>, squared_deviations >
::operator()( Derivatives& p, const OuterJacobian& dx, const DataRow& r )
{
    typename Data::Output values;

    evaluator.prepare_chunk( r.inputs );
    evaluator.value( values );
    r.residues = r.output - values;
    p.value += r.residues.square().sum();

    /* Compute the Y parts of the derivatives by part. */
    nonlinfit::Jacobian<Num,1,InnerTerms> dy;
    dy.compute( evaluator );

    gradient_accum +=
        ((dx->transpose() * r.residues.matrix()).array()
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

    if ( ! evaluator.prepare_iteration( *data ) )
        return false;
    
    /* Pre-compute the outer jacobian here so it will be available to compute
     * the residues in the inner loop. */
    OuterJacobian dx;
    dx.compute( evaluator );

    std::for_each( data->chunk_view().begin(), data->chunk_view().end(), 
        boost::bind( boost::ref(*this), boost::ref(p), boost::cref(dx), boost::arg<1>() ) );

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
