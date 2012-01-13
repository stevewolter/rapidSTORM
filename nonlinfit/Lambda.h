#ifndef NONLINFIT_EXPRESSION_H
#define NONLINFIT_EXPRESSION_H

namespace nonlinfit {

/** Concept for the description of a mathematical formula. A model of 
 *  Lambda defines a Boost.MPL sequence of variables and the semantics
 *  of their combination to a single number. In addition, an instance of a model
 *  of Lambda provides access to and storage for these parameters via 
 *  its operator()().
 *
 *  For example, a lambda LinearFunction for a function f(x) = a*x+b
 *  has three parameters (a, b and x). Its evaluation semantics are given 
 *  by the formula, and give a value of 5 for parameters a=2, x=2 and b=1.
 **/
template <typename Model>
class Lambda {
    struct Parameters;
};

}

#endif
