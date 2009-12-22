#ifndef ANDORCAMERA_TRIGGERING_H
#define ANDORCAMERA_TRIGGERING_H

#include <AndorCamera/StateMachine.h>
#include <simparm/NumericEntry.hh>
#include <AndorCamera/Config.h>

namespace AndorCamera {                                                    

/** Class representing all triggering modes. Does nothing for now. */
class Triggering
: public simparm::Object,
  public StateMachine::Listener
{ 
  protected:
    StateMachine &sm;
    
    void registerNamedEntries() {}
  public:
    Triggering(StateMachine& sm) 
        : simparm::Object("Triggering", "Trigger options"), sm(sm) {}
    Triggering(const Triggering&c) 
        : simparm::Object(c), StateMachine::Listener(), sm(c.sm) {}
    ~Triggering() {}
    Triggering* clone() const { return new Triggering(*this); }
    Triggering& operator=(const Triggering&) { return *this; }

    void controlStateChanged(Phase, State, State );
};

}

#endif
