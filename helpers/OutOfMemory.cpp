#include "helpers/OutOfMemory.h"

namespace dStorm {

OutOfMemoryMessage::OutOfMemoryMessage(const std::string& name)
: simparm::Message( 
        "Out of memory",
        "Your system has insufficient memory for " + name + "."
        "The most common reason is a high resolution enhancement combined "
        "with a large image. Try reducing either the image size or the "
        "resolution enhancement." )
{
}

}
