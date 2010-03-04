#ifndef ANDORCAMERA_ShutterControl_H
#define ANDORCAMERA_ShutterControl_H

#include <AndorCamera/StateMachine.h>
#include <simparm/NumericEntry.hh>
#include <AndorCamera/Config.h>

namespace AndorCamera {                                                    

/** Class representing all triggering modes. Does nothing for now. */
class ShutterControl
: public simparm::Object,
  public StateMachine::StandardListener<ShutterControl>
{ 
  protected:
    StateMachine &sm;
    
    void registerNamedEntries() {}
  public:
    ShutterControl(StateMachine& sm) ;
    ShutterControl(const ShutterControl&c);
    ~ShutterControl();
    ShutterControl* clone() const { return new ShutterControl(*this); }

    template <int State> class Token;
};

}

#endif
