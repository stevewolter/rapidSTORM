#include "guf/create_evaluator.h"
#include "nonlinfit/plane/create_term.hpp"
#include "nonlinfit/AbstractFunction.h"

namespace dStorm {
namespace guf {

template <typename DataTag, typename Expression>
std::unique_ptr<nonlinfit::plane::Term<DataTag>> create_evaluator(Expression& expression, DataTag way) {
    return nonlinfit::plane::create_term(expression, way);
}

}
}
