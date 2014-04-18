#ifndef DSTORM_EXPRESSION_CONFIG_VARIABLE_LVALUE_H
#define DSTORM_EXPRESSION_CONFIG_VARIABLE_LVALUE_H

#include "CommandLine.h"
#include "expression/LValue.h"

namespace dStorm {
namespace expression {

namespace source { std::auto_ptr<LValue> make_variable_lvalue( const Variable&, const std::string&, const Parser& ); }
namespace config { std::auto_ptr<LValue> make_variable_lvalue( const Variable& ); }

}
}

#endif
