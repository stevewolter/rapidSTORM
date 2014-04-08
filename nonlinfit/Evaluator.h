#ifndef NONLINFIT_EVALUATOR_H
#define NONLINFIT_EVALUATOR_H

#include "nonlinfit/fwd.h"

namespace nonlinfit {

/** \class get_evaluator
 *  \brief Metafunction returning an Evaluator model for its parameters.
 *  This metafunction should be specialized by the code defining
 *  a model when evaluator-based function evaluation is desired.
 *  \tparam Expression The Expression which is defining the evaluator's semantics.
 *  \tparam Tag   A computation tag defining the implicit parameters. */
template <typename Expression, typename Tag>
struct get_evaluator;

/** Concept for computing the value and per-parameter derivatives of a Lambda.
 *
 *  A class modelling Evaluator contains concrete code for computing
 *  the value and the partial derivatives for a Lambda.
 *  Contrast this to the Lambda itself, which defines 
 *  its semantics (the value that should be computed), but cannot perform the 
 *  computation because the data type is unbound. While a Lambda is equally
 *  valid for integers and complex numbers, an Evaluator is not.
 *
 *  An Evaluator can be viewed as a code repository for implementations of
 *  Function models on a Lambda. Different functions will instantiate
 *  different derivative() methods from an evaluator and weight the results 
 *  differently. However,
 *  they delegate the knowledge about the concrete calculation of derivatives
 *  to the Evaluator model. */
template <typename Model>
struct Evaluator {
    struct Expression;

    template <typename Parameter>
    struct derivation_summand_count;

    template <typename Result> void value( Result& result );
    template <typename Result, typename Parameter>
        void derivative( Result, Parameter );
};

}

#endif
