#include "Triggering.h"
#include "SDK.h"

using namespace AndorCamera::States;
using namespace AndorCamera::Phases;

void AndorCamera::Triggering::controlStateChanged
    (Phase phase, State from, State to)

{
    if ( phase == Transition && from == lower_state( Acquiring ) && to == Acquiring )
        SDK::SetTriggerMode( SDK::Trigger::Internal );
}
