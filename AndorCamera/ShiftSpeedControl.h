#ifndef ANDORCAMERA_ShiftSpeedControl_H
#define ANDORCAMERA_ShiftSpeedControl_H

#include <AndorCamera/StateMachine.h>
#include <simparm/NumericEntry.hh>
#include <AndorCamera/Config.h>
#include <dStorm/UnitEntries/Hertz.h>
#include <dStorm/UnitEntries/TimeEntry.h>

namespace AndorCamera {                                                    

class _ShiftSpeedControl {
  public:
    /** The A/D converter channel with this depth will be selected
        *  by default. */
    simparm::UnsignedLongEntry adChannelDepth;
    /** The A/D converter channel that will be used by 
        *  acquisitions. */
    simparm::ChoiceEntry       adChannel;
    /** The vertical scan speed closest to this one will 
        *  be selected by default. */
    dStorm::FloatMicrosecondsEntry       desired_VS_Speed;
    /** The horizontal scan speed closest to this one will 
        *  be selected by default. */
    dStorm::FloatMegahertzEntry       desired_HS_Speed;

    _ShiftSpeedControl();
};

class ShiftSpeedControl 
: public simparm::Object,
  public _ShiftSpeedControl,
  public StateMachine::StandardListener<ShiftSpeedControl>,
  public simparm::Node::Callback
{
    StateMachine &sm;
    simparm::DataChoiceEntry<OutputAmp>& outputAmp;
    simparm::DataChoiceEntry<int>& VS_Speed, HS_Speed;

    void fillADChannel();
    void fillHSSpeed();
    void fillVSSpeed();
    void registerNamedEntries();
    class ManagedAcquisition;
  public:
    ShiftSpeedControl(StateMachine &sm, Config& conf);
    ShiftSpeedControl(const ShiftSpeedControl&c);
    ~ShiftSpeedControl();

    void operator()(const simparm::Event&);
    template <int State> class Token;
};

}

#endif
