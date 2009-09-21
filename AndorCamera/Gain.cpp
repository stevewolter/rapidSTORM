#include "Gain.h"
#include "SDK.h"

namespace AndorCamera {

using namespace States;
using namespace Phases;

Gain::Gain(StateMachine& sm, Config &config) 
: simparm::Object("Gain", "Trigger options"), sm(sm),
    emccdGain(config.emccdGain) 
{ 
    emccdGain = 200; 
    push_back( emccdGain );
}

void Gain::controlStateChanged(
    Phase phase, State from, State to
) {
    if ( phase == Review && from == Disconnected ) {
        std::pair<int,int> gainRange = SDK::GetEMCCDGainRange();
        emccdGain.setMin( gainRange.first );
        emccdGain.setMax( gainRange.second );
    } else if ( phase == Transition && from == lower_state(Acquiring) && to == Acquiring ) {
        emccdGain.editable = false;
        SDK::SetEMCCDGain( emccdGain() );
    } else if ( phase == Transition && from == Acquiring && to == lower_state(Acquiring) ) {
        emccdGain.editable = true;
    }
}

}
