#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_EVALUATE_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_EVALUATE_H

#include "nonlinfit/plane/Distance.h"

#include "nonlinfit/plane/sum_matrix_rows.h"
#include <boost/bind/bind.hpp>

namespace nonlinfit {
namespace plane {

template <typename Tag, typename _Metric, int VariableCount>
bool Distance<Tag,_Metric,VariableCount>::evaluate(Derivatives& p)
{
    p.set_zero();

    for (const auto& term : terms) {
        if ( ! term->prepare_iteration( *xs ) ) {
            return false;
        }
    }
    
    for (auto i = this->xs->data.begin(); i != this->xs->data.end(); ++i) {
        Eigen::Array<Number, Tag::ChunkSize, 1> values(i->background);

        int offset = 0;
        for (const auto& term : terms) {
            auto block = jacobian.middleCols(offset, term->variable_count);
            term->evaluate_chunk(i->inputs, values, block);
            offset += term->variable_count;
        }
        assert(offset == jacobian.cols());

        i->residues = i->output - values;
        if (boost::is_same<_Metric, squared_deviations>::value) {
            p.value += i->residues.square().sum();
            p.hessian.noalias() += jacobian.transpose() * jacobian;
            p.gradient.noalias() += jacobian.transpose() * i->residues.matrix();
        } else {
            p.value -= 2 * (i->residues - i->logoutput + values.log() * i->output).sum();
            typename DataRow::Output quotient = i->output / values;
            typename DataRow::Output square_quotient = quotient / values;
            if (Tag::ChunkSize == 1) {
                p.hessian.noalias() += jacobian.transpose() * square_quotient(0, 0) * jacobian;
            } else {
                p.hessian.noalias() += jacobian.transpose() * square_quotient.matrix().asDiagonal() * jacobian;
            }
            p.gradient.noalias() += jacobian.transpose() * (quotient - 1).matrix();
        }

        assert( p.value == p.value );
    }

    return true;
}

template <typename Tag, typename _Metric, int VariableCount>
void Distance<Tag,_Metric,VariableCount>::get_position( Position& p ) const {
    int offset = 0;
    for (const auto& term : terms) {
        term->get_position(p.segment(offset, term->variable_count));
        offset += term->variable_count;
    }
    assert(offset == p.rows());
}

template <typename Tag, typename _Metric, int VariableCount>
void Distance<Tag,_Metric,VariableCount>::set_position( const Position& p ) {
    int offset = 0;
    for (const auto& term : terms) {
        term->set_position(p.segment(offset, term->variable_count));
        offset += term->variable_count;
    }
    assert(offset == p.rows());
}

template <typename Tag, typename _Metric, int VariableCount>
bool Distance<Tag,_Metric,VariableCount>::step_is_negligible( const Position& from, const Position& to ) const {
    int offset = 0;
    for (const auto& term : terms) {
        if (!term->step_is_negligible(
                    from.segment(offset, term->variable_count),
                    to.segment(offset, term->variable_count))) {
            return false;
        }
        offset += term->variable_count;
    }
    assert(offset == from.rows());
    assert(offset == to.rows());

    return true;
}

template <typename Num, int _ChunkSize, typename P1, typename P2, int VariableCount>
void Distance<Disjoint<Num,_ChunkSize,P1,P2>, squared_deviations, VariableCount >
::evaluate_chunk( Derivatives& p, const DataRow& r )
{
    Eigen::Array<Number, _ChunkSize, 1> values(r.background);

    int offset = 0;
    for (const auto& term : terms) {
        auto block = y_jacobian_row.middleCols(offset, term->term_variable_count);
        term->evaluate_disjoint_chunk(r.inputs, values, block);
        offset += term->term_variable_count;
    }
    assert(offset == y_jacobian_row.cols());

    r.residues = r.output - values;
    p.value += r.residues.square().sum();

    x_gradient.noalias() = x_jacobian.transpose() * r.residues.matrix();
    gradient_accum.noalias() +=
        (x_gradient.array() * y_jacobian_row.transpose().array()).matrix();
    y_hessian.noalias() += y_jacobian_row.transpose() * y_jacobian_row;

    assert( p.value == p.value );
}
template <typename Num, int _ChunkSize, typename P1, typename P2, int VariableCount>
bool Distance<Disjoint<Num,_ChunkSize,P1,P2>, squared_deviations, VariableCount >
::evaluate( Derivatives& p) 
{
    p.value = 0;
    gradient_accum.fill(0);
    y_hessian.fill(0);

    int offset = 0;
    for (const auto& term : terms) {
        auto block = x_jacobian.middleCols(offset, term->term_variable_count);
        if ( ! term->prepare_disjoint_iteration(*xs, block) ) {
            return false;
        }
        offset += term->term_variable_count;
    }
    assert(offset == x_jacobian.cols());
    
    for (auto i = this->xs->data.begin(); i != this->xs->data.end(); ++i) {
        evaluate_chunk(p, *i);
    }

    /* Compute the hessian matrix of derivation summands by multiplying
     * X and Y contributions. */
    x_hessian.noalias() = x_jacobian.transpose() * x_jacobian;
    hessian.noalias() = (x_hessian.array() * y_hessian.array()).matrix();

    sum_rows_and_cols(p.hessian, hessian, reduction);
    sum_rows(p.gradient, gradient_accum, reduction);

    return true;
}

template <typename Num, int _ChunkSize, typename P1, typename P2, int VariableCount>
void Distance<Disjoint<Num,_ChunkSize,P1,P2>, squared_deviations, VariableCount >::get_position( Position& p ) const {
    int offset = 0;
    for (const auto& term : terms) {
        term->get_position(p.segment(offset, term->variable_count));
        offset += term->variable_count;
    }
    assert(offset == p.rows());
}

template <typename Num, int _ChunkSize, typename P1, typename P2, int VariableCount>
void Distance<Disjoint<Num,_ChunkSize,P1,P2>, squared_deviations, VariableCount >::set_position( const Position& p ) {
    int offset = 0;
    for (const auto& term : terms) {
        term->set_position(p.segment(offset, term->variable_count));
        offset += term->variable_count;
    }
    assert(offset == p.rows());
}

template <typename Num, int _ChunkSize, typename P1, typename P2, int VariableCount>
bool Distance<Disjoint<Num,_ChunkSize,P1,P2>, squared_deviations, VariableCount >::step_is_negligible(
        const Position& from, const Position& to ) const {
    int offset = 0;
    for (const auto& term : terms) {
        if (!term->step_is_negligible(
                    from.segment(offset, term->variable_count),
                    to.segment(offset, term->variable_count))) {
            return false;
        }
        offset += term->variable_count;
    }
    assert(offset == from.rows());
    assert(offset == to.rows());

    return true;
}


}
}

#endif
