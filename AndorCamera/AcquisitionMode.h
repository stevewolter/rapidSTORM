#ifndef ANDORCAMERA_ACQUISITIONMODE_H
#define ANDORCAMERA_ACQUISITIONMODE_H

#include <AndorCamera/StateMachine.h>
#include <simparm/NumericEntry.hh>
#include <simparm/ChoiceEntry.hh>
#include <AndorCamera/Config.h>

namespace AndorCamera {                                                    

/** Class holding configuration items for AcquisitionMode. */
class _AcquisitionMode {
  public:
    simparm::DataChoiceEntry< AcquisitionMode > select_mode;
    simparm::DoubleEntry        desired_exposure_time;
    simparm::DoubleEntry        desired_accumulate_cycle_time;
    simparm::DoubleEntry        real_accumulate_cycle_time;
    simparm::UnsignedLongEntry  number_of_accumulations;
    simparm::DoubleEntry        desired_kinetic_cycle_time;
    simparm::UnsignedLongEntry  kinetic_length;

    _AcquisitionMode();
};

/** Base class for acquisition modes. */
class AcquisitionModeControl
: public simparm::Object,
  public _AcquisitionMode,
  public StateMachine::Listener,
  public simparm::Node::Callback
{ 
  protected:
    StateMachine &sm;
    /** The real exposure time for each image. */
    simparm::DoubleEntry&      real_exposure_time;
    simparm::DoubleEntry& real_kinetic_cycle_time;

    bool exp_time_is_max, acc_time_is_max;
    
    void registerNamedEntries();
  public:
    AcquisitionModeControl(StateMachine&, Config&);
    AcquisitionModeControl(const AcquisitionModeControl&);
    ~AcquisitionModeControl();
    AcquisitionModeControl* clone() const { return new AcquisitionModeControl(*this); }
    AcquisitionModeControl& operator=(const AcquisitionModeControl&);

    void operator()(const simparm::Event&);
    void controlStateChanged(Phase phase, State from, State to);
};

}

#endif
