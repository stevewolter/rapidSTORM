#ifndef DSTORM_INPUT_SOURCE_IMPL_H
#define DSTORM_INPUT_SOURCE_IMPL_H

#include "Source.h"
#include "Drain.h"
#include "ImageTraits_decl.h"
#include "../error_handler.h"
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
: BaseSource(node, flags), pushTarget(NULL) 
{
}

template <typename Type>
void Source<Type>::startPushing(Drain<Type> *drain)

{
    pushTarget = drain;

    simparm::TriggerEntry stopPushing("StopReading", "Stop reading input");
    getNode().push_back( stopPushing );

    for (unsigned int i = roi_start; i <= roi_end; i++) {
        Type *object = fetch(i);
        if ( object == NULL )
            return;
        else {
            if ( ErrorHandler::global_termination_flag() || stopPushing.value() || pushTarget != drain ) break;
            Management m;
            m = drain->accept( i - roi_start, 1, object );
            if ( !manages_returned_objects() &&
                    m == Delete_objects )
                delete object;
        }
    }
}

}
}

#endif
