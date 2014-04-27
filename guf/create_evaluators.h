#ifndef DSTORM_GUF_CREATE_EVALUATORS_H
#define DSTORM_GUF_CREATE_EVALUATORS_H

#include <memory>
#include <vector>

#include "nonlinfit/plane/Term.h"

namespace dStorm {
namespace guf {

template <typename DataTag, typename Expression>
std::vector<std::unique_ptr<nonlinfit::plane::Term<DataTag>>> create_evaluators(Expression& expression, DataTag way);

}
}

#endif
