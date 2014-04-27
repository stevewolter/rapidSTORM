#include "guf/create_evaluators.hpp"

namespace dStorm {
namespace guf {

template 
std::vector<std::unique_ptr<nonlinfit::plane::Term<InstantiatedTag>>> create_evaluators<InstantiatedTag, InstantiatedFunction>(
    InstantiatedFunction& expression, InstantiatedTag way);

}
}
