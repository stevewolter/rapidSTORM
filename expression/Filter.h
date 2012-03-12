#ifndef DSTORM_EXPRESSION_CONFIG_FILTER_H
#define DSTORM_EXPRESSION_CONFIG_FILTER_H

#include "CommandLine.h"

namespace dStorm {
namespace expression {

namespace source { std::auto_ptr<LValue> make_filter( const std::string&, const Parser& ); }
namespace config { std::auto_ptr<LValue> make_filter(); }
}
}

#endif
