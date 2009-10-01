#ifndef ANDORCAMERA_ShutterControl_H
#define ANDORCAMERA_ShutterControl_H

#include <AndorCamera/StateMachine.h>
#include <simparm/NumericEntry.hh>
#include <AndorCamera/Config.h>

namespace AndorCamera {                                                    

/** Class representing all triggering modes. Does nothing for now. */
class ShutterControl
: public simparm::Object,
  public StateMachine::Listener
{ 
  protected:
    StateMachine &sm;
    bool hasMechanicalShutter;
    
    void registerNamedEntries() {}
  public:
    ShutterControl(StateMachine& sm) 
    : simparm::Object("ShutterControl", "Shutter control"), sm(sm) {}
    ShutterControl(const ShutterControl&c) 
    : simparm::Node(c), simparm::Object(c), StateMachine::Listener(), sm(c.sm) {}
    ~ShutterControl() {}
    ShutterControl* clone() const { return new ShutterControl(*this); }
    ShutterControl& operator=(const ShutterControl&) { return *this; }

    void controlStateChanged(Phase phase, State from, State to);
};

}

#endif
