#ifndef ANDORCAMERA_TEMPERATURE_H
#define ANDORCAMERA_TEMPERATURE_H

#include <AndorCamera/StateMachine.h>
#include <simparm/NumericEntry.hh>
#include <AndorCamera/Config.h>

namespace AndorCamera {                                                    

class _Temperature {
  public:
    /** The temperature that is required to be reached before the 
        *  shutter may be opened and acquisitions started. */
    simparm::LongEntry         requiredTemperature;
    /** The current CCD temperature. Is only updated while the 
        *  system is cooling. */
    simparm::DoubleEntry       realTemperature;
    /** /brief Controls and displays the cooling of the
        *  camera
        *
        *  If this element is set to true externally, the camera
        *  goes to the Cooling state; if it is set to false,
        *  to the Initialized state. If the camera goes to one
        *  of these states through program-internal processes 
        *  (for example, automatic acquisition preparation),
        *  doCool is updated to match the state. */
    simparm::BoolEntry         doCool;

    _Temperature();
};

class TemperatureMonitor;

class Temperature 
: public simparm::Object,
  public _Temperature,
  public StateMachine::Listener,
  public simparm::Node::Callback
{
    StateMachine &sm;
    simparm::LongEntry& targetTemperature;
    std::auto_ptr<TemperatureMonitor> monitor;
    bool am_cooling;

    void registerNamedEntries();
    void cool();
    void warm();
  public:
    Temperature(StateMachine &sm, Config& conf);
    Temperature(const Temperature&c);
    ~Temperature();

    void operator()(Node &, Cause, Node *);
    void controlStateChanged(Phase phase, State from, State to);
};

}

#endif
