#ifndef ANDORCAMERA_Gain_H
#define ANDORCAMERA_Gain_H

#include <AndorCamera/StateMachine.h>
#include <simparm/NumericEntry.hh>
#include <AndorCamera/Config.h>

namespace AndorCamera {                                                    

/** Class managing gain settings. */
class Gain
: public simparm::Object,
  public StateMachine::Listener
{ 
  protected:
    StateMachine &sm;
    
    void registerNamedEntries() {}
  public:
    simparm::NumericEntry<int>& emccdGain;

    Gain(StateMachine& sm, Config &config);
    Gain(const Gain&c) 
        : simparm::Object(c), StateMachine::Listener(), sm(c.sm),
          emccdGain(c.emccdGain) {}
    ~Gain() {}
    Gain* clone() const { return new Gain(*this); }
    Gain& operator=(const Gain&) { return *this; }

    void controlStateChanged(Phase, State, State );
};

}

#endif
