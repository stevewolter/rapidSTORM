#include "guf/create_evaluators.h"
#include "nonlinfit/plane/create_term.hpp"
#include "nonlinfit/sum/Evaluator.h"
#include "nonlinfit/AbstractFunction.h"

namespace dStorm {
namespace guf {

template <typename DataTag, typename Expression>
std::vector<std::unique_ptr<nonlinfit::plane::Term<DataTag>>> create_evaluators(Expression& expression, DataTag way) {
    std::vector<std::unique_ptr<nonlinfit::plane::Term<DataTag>>> evaluators;
    evaluators.push_back(create_term(expression, way));
    return evaluators;
}

}
}
