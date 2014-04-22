#ifndef NONLINFIT_PLANE_CHECK_EVALUATOR_HPP
#define NONLINFIT_PLANE_CHECK_EVALUATOR_HPP

#include "nonlinfit/plane/fwd.h"
#include <nonlinfit/Lambda.h>
#include <nonlinfit/Evaluation.hpp>
#include <nonlinfit/BoundFunction.hpp>
#include <nonlinfit/plane/JointData.hpp>
#include <nonlinfit/plane/Distance.hpp>
#include <cassert>

namespace nonlinfit {
namespace plane {

/** Tests whether two tagged invocations of Distance yield the same result. */
template <
    class Metric,
    class Tag,
    class RefTag,
    class Function>
bool compare_evaluators( 
    const Function& model,
    const typename Tag::Data& data
)
{
    BoundFunction< Distance< Function, Tag, Metric > > test;
    BoundFunction< Distance< Function, RefTag, Metric > > ref;
    test.get_data() = data;
    ref.get_data() = data;
    test.get_expression() = model;
    ref.get_expression() = test.get_expression();

    typedef typename Tag::Number Num;
    Evaluation<Num> test_result(test.variable_count()), reference_result(ref.variable_count());
    test.evaluate( test_result );
    ref.evaluate( reference_result );

    assert(( ! test_result.contains_NaN() ));
    assert(( ! reference_result.contains_NaN() ));
    if (!(test_result == reference_result)) {
      std::cerr << "Test result " << test_result << " against reference " << reference_result << std::endl;
    }
    return test_result == reference_result;
}

}
}

#endif
