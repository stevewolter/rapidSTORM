#ifndef DSTORM_OUT_OF_MEMORY_H
#define DSTORM_OUT_OF_MEMORY_H

#include <simparm/Message.h>

namespace dStorm {

class OutOfMemoryMessage : public simparm::Message {
  public:
    OutOfMemoryMessage(const std::string& name);
};

}

#endif
