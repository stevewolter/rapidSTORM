#include "debug.h"
#include "AcquisitionSwitch.h"
#include "StateMachine_impl.h"
#include "SDK.h"

namespace AndorCamera {

MK_EMPTY_RW(AcquisitionSwitch)

template <>
class AcquisitionSwitch::Token<States::Acquiring> 
: public States::Token
{
    AcquisitionSwitch& parent;
  public:
    Token(AcquisitionSwitch& s) 
    : parent(s)
    {
        if ( s.is_active ) {
            s.sm.status = "Acquiring images";
            SDK::StartAcquisition();
        }
    }

    ~Token() 
    {
        if ( parent.is_active )
            while ( SDK::GetStatus() == SDK::Is_Acquiring ) {
                DEBUG("Waiting for acquisition to terminate");
                try {
                    SDK::AbortAcquisition();
                } catch ( const Error& ) {}
                try {
                    SDK::WaitForAcquisition();
                } catch ( const Error& ) {}
            }
    }
};

AcquisitionSwitch::AcquisitionSwitch(StateMachine& sm)
        : StateMachine::StandardListener<AcquisitionSwitch>(*this),
          sm(sm) {}

template class StateMachine::StandardListener<AcquisitionSwitch>;

}
