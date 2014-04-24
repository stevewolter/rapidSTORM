#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_EVALUATE_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_EVALUATE_H

#include "nonlinfit/plane/Distance.h"

#include "nonlinfit/plane/sum_matrix_rows.h"
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

template <typename Tag, typename _Metric>
void Distance<Tag,_Metric>::evaluate_chunk( 
    Derivatives& p, const DataRow& r, const DataChunk& c )
{
}


template <typename Tag, typename _Metric>
bool Distance<Tag,_Metric>::evaluate(Derivatives& p)
{
    p.set_zero();

    for (const auto& term : terms) {
        if ( ! term->prepare_iteration( *xs ) ) {
            return false;
        }
    }
    
    assert(this->xs->data.size() == this->ys->size());
    Eigen::Array<Number, Tag::ChunkSize, 1> values;
    auto j = this->ys->begin();
    for (auto i = this->xs->data.begin(); i != this->xs->data.end(); ++i, ++j) {
        values.fill(0);

        int offset = 0;
        for (const auto& term : terms) {
            term->evaluate_chunk(i->inputs, values,
                    jacobian.template block<Tag::ChunkSize, Eigen::Dynamic>(
                        0, offset, Tag::ChunkSize, term->variable_count));
            offset += term->variable_count;
        }

        j->residues = j->output - values;
        increment_evaluation< _Metric >()( p, values, *j, jacobian );
        assert( p.value == p.value );
    }

    return true;
}

template <typename Tag, typename _Metric>
void Distance<Tag,_Metric>::get_position( Position& p ) const {
    int offset = 0;
    for (const auto& term : terms) {
        term->get_position(p.segment(offset, term->variable_count));
        offset += term->variable_count;
    }
}

template <typename Tag, typename _Metric>
void Distance<Tag,_Metric>::set_position( const Position& p ) {
    int offset = 0;
    for (const auto& term : terms) {
        term->set_position(p.segment(offset, term->variable_count));
        offset += term->variable_count;
    }
}

template <typename Num, int _ChunkSize, typename P1, typename P2>
void Distance<Disjoint<Num,_ChunkSize,P1,P2>, squared_deviations >
::evaluate_chunk( Derivatives& p, const DataRow& r, const DataChunk& c )
{
    Eigen::Array<Number, _ChunkSize, 1> values;
    values.fill(0);

    int offset = 0;
    for (const auto& term : terms) {
        term->evaluate_disjoint_chunk(r.inputs, values,
                y_jacobian_row.template block<1, Eigen::Dynamic>(
                    0, offset, 1, term->term_variable_count));
        offset += term->term_variable_count;
    }

    c.residues = c.output - values;
    p.value += c.residues.square().sum();

    gradient_accum +=
        ((x_jacobian.transpose() * c.residues.matrix()).array()
            * y_jacobian_row.transpose().array()).matrix();
    y_hessian += y_jacobian_row.transpose() * y_jacobian_row;

    assert( p.value == p.value );
}
template <typename Num, int _ChunkSize, typename P1, typename P2>
bool Distance<Disjoint<Num,_ChunkSize,P1,P2>, squared_deviations >
::evaluate( Derivatives& p) 
{
    p.value = 0;
    gradient_accum.fill(0);
    y_hessian.fill(0);

    int offset = 0;
    for (const auto& term : terms) {
        auto block = x_jacobian.template block<_ChunkSize, Eigen::Dynamic>(
                0, offset, _ChunkSize, term->term_variable_count);
        if ( ! term->prepare_disjoint_iteration(*xs, block) ) {
            return false;
        }
        offset += term->term_variable_count;
    }
    
    assert(this->xs->data.size() == this->ys->size());
    auto j = this->ys->begin();
    for (auto i = this->xs->data.begin(); i != this->xs->data.end(); ++i, ++j) {
        evaluate_chunk(p, *i, *j);
    }

    /* Compute the hessian matrix of derivation summands by multiplying
     * X and Y contributions. */
    x_hessian = x_jacobian.transpose() * x_jacobian;

    sum_rows_and_cols(p.hessian, (x_hessian.array() * y_hessian.array()).matrix(), reduction);
    sum_rows(p.gradient, gradient_accum, reduction);

    return true;
}

template <typename Num, int _ChunkSize, typename P1, typename P2>
void Distance<Disjoint<Num,_ChunkSize,P1,P2>, squared_deviations >::get_position( Position& p ) const {
    int offset = 0;
    for (const auto& term : terms) {
        term->get_position(p.segment(offset, term->term_variable_count));
        offset += term->term_variable_count;
    }
}

template <typename Num, int _ChunkSize, typename P1, typename P2>
void Distance<Disjoint<Num,_ChunkSize,P1,P2>, squared_deviations >::set_position( const Position& p ) {
    int offset = 0;
    for (const auto& term : terms) {
        term->set_position(p.segment(offset, term->term_variable_count));
        offset += term->term_variable_count;
    }
}


}
}

#endif
