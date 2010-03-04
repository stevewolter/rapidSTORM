#include "ShutterControl.h"
#include "SDK.h"
#include "StateMachine_impl.h"

namespace AndorCamera {

ShutterControl::ShutterControl(StateMachine& sm) 
    : simparm::Object("ShutterControl", "Shutter control"),
      StateMachine::StandardListener<ShutterControl>(*this),
      sm(sm) {}
ShutterControl::ShutterControl(const ShutterControl&c) 
    : simparm::Object(c), 
      StateMachine::StandardListener<ShutterControl>(*this),
      sm(c.sm) {}
ShutterControl::~ShutterControl() {}

MK_EMPTY_RW(ShutterControl)

template <>
class ShutterControl::Token<States::Acquiring>
: public States::Token
{
  public:
    Token(ShutterControl&) {
        SDK::SetShutter( SDK::Shutter::High, SDK::Shutter::Open, 10, 10 );
    }
    ~Token() {
        SDK::SetShutter( SDK::Shutter::High, SDK::Shutter::Closed, 10, 10 );
    }
};

template class StateMachine::StandardListener<ShutterControl>;

}
