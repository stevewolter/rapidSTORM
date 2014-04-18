#ifndef DSTORM_EXPRESSION_LOCALIZATION_VARIABLE_DECL_H
#define DSTORM_EXPRESSION_LOCALIZATION_VARIABLE_DECL_H

#include <boost/ptr_container/ptr_vector.hpp>
#include <memory>
#include "expression/Variable.h"

namespace dStorm {
namespace expression {

std::auto_ptr< boost::ptr_vector<Variable> >
    variables_for_localization_fields();

}
}

#endif
