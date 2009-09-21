#ifndef CIMGBUFFER_SOURCE_IMPL_H
#define CIMGBUFFER_SOURCE_IMPL_H

#include <CImgBuffer/Source.h>

namespace CImgBuffer {
template <typename Type>
Source<Type>::Source(const BaseSource::Flags& flags)
: simparm::Node(), BaseSource(flags),
    pushTarget(NULL) 
{
}
}

#endif
