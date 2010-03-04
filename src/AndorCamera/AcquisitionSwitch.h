#ifndef ANDORCAMERA_ACQUISITIONSWITCH_H
#define ANDORCAMERA_ACQUISITIONSWITCH_H

#include "StateMachine.h"

namespace AndorCamera {

class AcquisitionSwitch 
: public StateMachine::StandardListener<AcquisitionSwitch>
{
    StateMachine& sm;
  public:
    AcquisitionSwitch(StateMachine& sm);
    template <int State> class Token;
};

}

#endif
