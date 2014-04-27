#include "guf/create_evaluator.hpp"

namespace dStorm {
namespace guf {

template 
std::unique_ptr<nonlinfit::plane::Term<InstantiatedTag>>
create_evaluator<InstantiatedTag, InstantiatedExpression>(
    InstantiatedExpression& expression, InstantiatedTag way);

}
}
