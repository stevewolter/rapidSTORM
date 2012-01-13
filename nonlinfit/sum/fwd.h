#ifndef NONLINFIT_SUM_FWD_H
#define NONLINFIT_SUM_FWD_H

namespace nonlinfit {

/** Represent the sum of Lambda or Function models. */
namespace sum {

template <typename Parts>
class Lambda;
template <typename Parts, typename ComputationTag>
class Evaluator;

template <int VariableCount>
class AbstractMap;

}

}

#endif
