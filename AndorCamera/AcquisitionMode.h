#ifndef ANDORCAMERA_ACQUISITIONMODE_H
#define ANDORCAMERA_ACQUISITIONMODE_H

#include <AndorCamera/StateMachine.h>
#include <simparm/NumericEntry.hh>
#include <simparm/ChoiceEntry.hh>
#include <AndorCamera/Config.h>
#include <dStorm/UnitEntries/TimeEntry.h>
#include <dStorm/UnitEntries/FrameEntry.h>
#include <simparm/OptionalEntry.hh>

namespace AndorCamera {                                                    

/** Class holding configuration items for AcquisitionMode. */
class _AcquisitionMode {
  public:
    simparm::DataChoiceEntry< AcquisitionMode > select_mode;
    dStorm::FloatTimeEntry      desired_exposure_time;
    dStorm::FloatTimeEntry      desired_accumulate_cycle_time;
    dStorm::FloatTimeEntry      real_accumulate_cycle_time;
    simparm::UnsignedLongEntry  number_of_accumulations;
    dStorm::FloatTimeEntry      desired_kinetic_cycle_time;
    simparm::Selector< simparm::optional< boost::units::quantity<
        cs_units::camera::time, int > > >::Entry kinetic_length;

    _AcquisitionMode();
};

/** Base class for acquisition modes. */
class AcquisitionModeControl
: public simparm::Object,
  public _AcquisitionMode,
  public StateMachine::StandardListener<AcquisitionModeControl>,
  public simparm::Node::Callback
{ 
  protected:
    StateMachine &sm;
    /** The real exposure time for each image. */
    dStorm::FloatTimeEntry& real_exposure_time;
    dStorm::FloatTimeEntry& real_kinetic_cycle_time;

    bool exp_time_is_max, acc_time_is_max;
    
    void registerNamedEntries();
    class ManagedAcquisition;
  public:
    AcquisitionModeControl(StateMachine&, Config&);
    AcquisitionModeControl(const AcquisitionModeControl&);
    ~AcquisitionModeControl();
    AcquisitionModeControl* clone() const { return new AcquisitionModeControl(*this); }
    AcquisitionModeControl& operator=(const AcquisitionModeControl&);

    void operator()(const simparm::Event&);
    template <int State> class Token;
};

}

#endif
