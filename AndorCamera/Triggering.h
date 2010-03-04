#ifndef ANDORCAMERA_TRIGGERING_H
#define ANDORCAMERA_TRIGGERING_H

#include <AndorCamera/StateMachine.h>
#include <simparm/NumericEntry.hh>
#include <AndorCamera/Config.h>

namespace AndorCamera {                                                    

/** Class representing all triggering modes. Does nothing for now. */
class Triggering
: public simparm::Object,
  public StateMachine::StandardListener<Triggering>
{ 
  protected:
    StateMachine &sm;
    
    void registerNamedEntries() {}
  public:
    Triggering(StateMachine& sm);
    Triggering(const Triggering&c);
    ~Triggering();
    Triggering* clone() const { return new Triggering(*this); }

    template <int State> class Token;
};

}

#endif
