#ifndef NONLINFIT_PLANE_TERM_H
#define NONLINFIT_PLANE_TERM_H

#include "nonlinfit/plane/DisjointTermImplementation.h"
#include "nonlinfit/plane/JointTermImplementation.h"

namespace nonlinfit {
namespace plane {

template <typename Expression, typename Num, int ChunkSize, typename FirstParam, typename SecondParam>
std::unique_ptr<Term<Joint<Num, ChunkSize, FirstParam, SecondParam>>> create_term(
        Expression& expression, Joint<Num, ChunkSize, FirstParam, SecondParam> data_tag) {
    return std::unique_ptr<Term<Joint<Num, ChunkSize, FirstParam, SecondParam>>>(
            new JointTermImplementation<Expression, Joint<Num, ChunkSize, FirstParam, SecondParam>>(expression));
}

template <typename Expression, typename Num, int ChunkSize, typename OuterParam, typename InnerParam>
std::unique_ptr<Term<Disjoint<Num, ChunkSize, OuterParam, InnerParam>>> create_term(
        Expression& expression, Disjoint<Num, ChunkSize, OuterParam, InnerParam> data_tag) {
    return std::unique_ptr<Term<Disjoint<Num, ChunkSize, OuterParam, InnerParam>>>(
            new DisjointTermImplementation<Expression, Disjoint<Num, ChunkSize, OuterParam, InnerParam>>(expression));
}

}
}

#endif
