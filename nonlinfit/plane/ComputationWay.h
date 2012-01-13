#ifndef NONLINFIT_PLANE_COMPUTATION_WAY_H
#define NONLINFIT_PLANE_COMPUTATION_WAY_H

namespace nonlinfit {
namespace plane {

/** Concept for a method of computation compatible with FunctionTemplate.
 *  \sa FunctionTemplate, Joint, Disjoint */
template <typename Model>
struct ComputationWay {
    /** The type of the evaluated number, e.g. double or float. */
    struct Number;
    /** The type of data that is used in this computation way. The data must
     *  be a model of a STL Container. */
    struct Data;
    /** The row count of the derivative matrix for one data row. */
    static const int InnerDataWidth = 0;
    /** A Boost.MPL metafunction returning the sequence of derivative summands
     *  with respect to the given data dimension. A derivative summand is either
     *  an instance of nonlinfit::DerivationSummand or a parameter tag. */
    template <typename Function, typename Parameters>
    struct make_derivative_terms;
};

}
}

#endif
