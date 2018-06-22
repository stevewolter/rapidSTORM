#ifndef NONLINFT_FWD_H
#define NONLINFT_FWD_H

#include "nonlinfit/sum/fwd.h"
#include "nonlinfit/levmar/fwd.h"
#include "nonlinfit/terminators/fwd.h"

/** The nonlinfit namespace contains a generic nonlinear fitting
 *  header library. Its core concepts are 
 *  - nonlinfit::Lambda
 *  - nonlinfit::Evaluator
 *  - nonlinfit::Function
 *
 **/
namespace nonlinfit {

template <typename Model> struct Lambda;
template <typename Model> struct Evaluator;
template <typename Model> struct Function;
template <class X, class Position> struct Terminator;

template <class Derivatives>
    class AbstractFunction;
template <typename Lambda, typename Assignment> 
    struct Bind;
template <class _Summand, typename _Parameter, class _Dimension> 
    struct DerivationSummand;
template <class Num> 
    struct Evaluation;
template <typename Lambda> 
    struct VectorPosition;
template <typename Func, typename BaseParameter> 
    struct TermParameter;

template <typename Lambda, typename Tag> 
    struct get_evaluator;

template <typename Parameter> class ParameterReference;
template <typename CRTP> struct access_parameters;
template <typename A, typename B> struct append;
template <typename Function> class BoundFunction;
template <class Num, int VectorSize, typename _Variables> class Jacobian;
template <typename List, typename Parameter> struct index_of;
template <class Lambda, class Result> struct make_functor;

}

#endif
