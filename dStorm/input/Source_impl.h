#ifndef DSTORM_INPUT_SOURCE_IMPL_H
#define DSTORM_INPUT_SOURCE_IMPL_H

#include "Source.h"
#include "Drain.h"
#include <simparm/TriggerEntry.hh>

namespace dStorm {
namespace input {

#if 0 /* Enable when C++0x arrives */
extern template class Source< cimg_library::CImg<unsigned int> >;
extern template class Source< cimg_library::CImg<unsigned short> >;
extern template class Source< cimg_library::CImg<unsigned char> >;
extern template class Source< cimg_library::CImg<float> >;
#endif

template <typename Type>
Source<Type>::Source(simparm::Node& node, const BaseSource::Flags& flags)
: BaseSource(node, flags)
{
}

#if 0
template <typename Type>
void Source<Type>::startPushing(Drain<Type> *drain)

{
    pushTarget = drain;

    simparm::TriggerEntry stopPushing("StopReading", "Stop reading input");
    getNode().push_back( stopPushing );

    typename Drain<Type>::iterator o = drain->begin();
    iterator i_end = end();
    for ( iterator i = begin(); i != i_end; i++ ) {
        if ( ErrorHandler::global_termination_flag() || stopPushing.value() || pushTarget != drain ) break;
        *o++ = *i;
    }
}
#endif

}
}

#endif
