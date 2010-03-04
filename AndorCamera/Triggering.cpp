#include "Triggering.h"
#include "SDK.h"
#include "StateMachine_impl.h"

using namespace AndorCamera::States;

namespace AndorCamera {

Triggering::Triggering(StateMachine& sm)
    : simparm::Object("Triggering", "Trigger options"), 
      StateMachine::StandardListener<Triggering>(*this),
      sm(sm) {}
Triggering::Triggering(const Triggering&c)
    : simparm::Object(c), 
      StateMachine::StandardListener<Triggering>(*this),
      sm(c.sm) {}
Triggering::~Triggering() {}

MK_EMPTY_RW(Triggering)

template <>
class Triggering::Token<States::Readying>
: public States::Token
{
  public:
    Token(Triggering&) {
        SDK::SetTriggerMode( SDK::Trigger::Internal );
    }
};

template class StateMachine::StandardListener<Triggering>;


}
