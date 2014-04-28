#ifndef NONLINFT_FWD_H
#define NONLINFT_FWD_H

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

template <class Derivatives>
    class AbstractFunction;
template <typename Lambda, typename Assignment> 
    struct Bind;
template <class _Summand, typename _Parameter, class _Dimension> 
    struct DerivationSummand;
template <class Num> 
    struct Evaluation;

template <typename Lambda, typename Tag> 
    struct get_evaluator;

template <typename Parameter> class ParameterReference;
template <typename CRTP> struct access_parameters;
template <typename A, typename B> struct append;
template <typename Function> class BoundFunction;
template <typename List, typename Parameter> struct index_of;
template <class Lambda, class Result> struct make_functor;

}

#endif
