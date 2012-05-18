#ifndef DSTORM_ENGINE_OUTPUTBUILDER_IMPL_H
#define DSTORM_ENGINE_OUTPUTBUILDER_IMPL_H

#include "OutputBuilder.h"

namespace dStorm {
namespace output {

template <typename Config, typename Output>
OutputBuilder<Config, Output>::OutputBuilder(
    bool failSilently)
: failSilently("FailSilently", 
        "Allow transmission to fail silently",
        failSilently),
  name_object( Config::get_name(), Config::get_description() )
{ 
    this->failSilently.userLevel = simparm::Object::Debug;
}

}
}

#endif
