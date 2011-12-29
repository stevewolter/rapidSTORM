#include "Expression.h"

namespace dStorm {
namespace expression {



struct Boolean::Impl {
};

Boolean::Boolean(std::string expression)
{
}

bool Boolean::evaluate(const Localization&) const
{
    return true;
}

}
}
