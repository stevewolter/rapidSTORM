#ifndef DSTORM_ENGINE_OUTPUTBUILDER_IMPL_H
#define DSTORM_ENGINE_OUTPUTBUILDER_IMPL_H

#include "OutputBuilder.h"

namespace dStorm {
namespace output {

template <typename Type>
OutputBuilder<Type>::OutputBuilder(
    bool failSilently)
: OutputSource(static_cast<typename Type::Config&>(*this)),
  failSilently("FailSilently", 
        "Allow transmission to fail silently",
        failSilently)
{ 
    this->failSilently.userLevel = simparm::Object::Debug;
    push_back( this->failSilently );
    push_back( this->help_file ); 
}

template <typename Type>
OutputBuilder<Type>::OutputBuilder(const OutputBuilder& o)
: Type::Config(o),
    OutputSource(
        static_cast<typename Type::Config&>(*this), o),
    failSilently(o.failSilently)
{ 
    this->push_back( failSilently ); 
    this->push_back( this->help_file ); 
}

}
}

#endif
