#ifndef DSTORM_INPUT_SERIALSOURCE_H
#define DSTORM_INPUT_SERIALSOURCE_H

#include "Source.h"
#include "Drain.h"
#include <dStorm/helpers/thread.h>
#include <cassert>

namespace dStorm {
namespace input {

/** Base class for sources where serial access to data is desired.
 *  Sources that prefer objects to be fetched in sequence (0,1,2,3
 *  instead of 1,0,3,2) should derive from this class and overload
 *  load() instead of deriving from Source. */
template <typename Type>
class SerialSource 
    : public Source< Type >
{
    /** The serializer controls multi-threaded access to the
     *  source. */
    ost::Mutex serializer;
    int current_object;
  protected:
    SerialSource( simparm::Node& node, int flags ) 
        : Source< Type >(node, flags), current_object(-1) {}
    virtual Type* load() = 0;
    virtual Type* fetch(int image_index);
};

}
}

#endif 
