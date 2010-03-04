#ifndef ANDORCAMERA_Gain_H
#define ANDORCAMERA_Gain_H

#include <AndorCamera/StateMachine.h>
#include <simparm/NumericEntry.hh>
#include <AndorCamera/Config.h>

namespace AndorCamera {                                                    

/** Class managing gain settings. */
class Gain
: public simparm::Object,
  public StateMachine::StandardListener<Gain>
{ 
  protected:
    StateMachine &sm;
    
    void registerNamedEntries() {}
  public:
    simparm::NumericEntry<int>& emccdGain;

    Gain(StateMachine& sm, Config &config);
    Gain* clone() const { return new Gain(*this); }

    template <int State> class Token;
};

}

#endif
