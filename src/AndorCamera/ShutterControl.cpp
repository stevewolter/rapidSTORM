#include "ShutterControl.h"
#include "SDK.h"

namespace AndorCamera {

using namespace Phases;
using namespace States;

void ShutterControl::controlStateChanged(Phase phase, State from, State to)

{
    if ( phase == Review && from == Disconnected )
        hasMechanicalShutter = SDK::IsInternalMechanicalShutter();
    else if ( phase == Transition && to == Acquiring ) {
        SDK::SetShutter( SDK::Shutter::High, SDK::Shutter::Open, 10, 10 );
    } else if ( phase == Transition && from == Acquiring ) {
        SDK::SetShutter( SDK::Shutter::High, SDK::Shutter::Closed, 10, 10 );
    }
}

}
