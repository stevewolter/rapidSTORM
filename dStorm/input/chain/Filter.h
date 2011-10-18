#ifndef DSTORM_INPUT_CHAIN_FILTER_H
#define DSTORM_INPUT_CHAIN_FILTER_H

#include "Forwarder.h"
#include "../Traits.h"

namespace dStorm {
namespace input {
namespace chain {

class Filter : public Forwarder {
  public:
    ~Filter() {}
    Filter* clone() const = 0;
};

template <typename Type>
class DefaultVisitor;

class DelegateToVisitor;
class DefaultTypes;

}
}
}

#endif
