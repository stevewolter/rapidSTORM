#ifndef DSTORM_ENGINE_OUTPUTBUILDER_IMPL_H
#define DSTORM_ENGINE_OUTPUTBUILDER_IMPL_H

#include "OutputBuilder.h"

namespace dStorm {
namespace output {

template <typename Type, typename BaseSource>
OutputBuilder<Type,BaseSource>::OutputBuilder(
    bool failSilently)
: BaseSource(static_cast<typename Type::Config&>(*this)),
  failSilently("FailSilently", 
        "Allow transmission to fail silently",
        failSilently)
{ 
    this->failSilently.userLevel = simparm::Object::Debug;
    this->push_back( this->failSilently );
}

template <typename Type, typename BaseSource>
OutputBuilder<Type,BaseSource>::OutputBuilder(const OutputBuilder& o)
: Type::Config(o),
    BaseSource(
        static_cast<typename Type::Config&>(*this), o),
    failSilently(o.failSilently)
{ 
    this->push_back( failSilently ); 
}

}
}

#endif
