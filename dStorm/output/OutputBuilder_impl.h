#ifndef DSTORM_ENGINE_OUTPUTBUILDER_IMPL_H
#define DSTORM_ENGINE_OUTPUTBUILDER_IMPL_H

#include "OutputBuilder.h"

namespace dStorm {
namespace output {

template <typename Type, typename BaseSource>
OutputBuilder<Type,BaseSource>::OutputBuilder(
    bool failSilently)
: failSilently("FailSilently", 
        "Allow transmission to fail silently",
        failSilently),
  name_object( Type::Config::getName(), Type::Config::getDesc() )
{ 
    this->failSilently.userLevel = simparm::Object::Debug;
}

template <typename Type, typename BaseSource>
OutputBuilder<Type,BaseSource>::OutputBuilder(const OutputBuilder& o)
: Type::Config(o),
  BaseSource(o),
  failSilently(o.failSilently),
  name_object( o.name_object )
{ 
}

}
}

#endif
