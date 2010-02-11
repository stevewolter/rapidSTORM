#ifndef ANDORCAMERA_ShiftSpeedControl_H
#define ANDORCAMERA_ShiftSpeedControl_H

#include <AndorCamera/StateMachine.h>
#include <simparm/NumericEntry.hh>
#include <AndorCamera/Config.h>

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
    simparm::DoubleEntry       desired_VS_Speed;
    /** The horizontal scan speed closest to this one will 
        *  be selected by default. */
    simparm::DoubleEntry       desired_HS_Speed;

    _ShiftSpeedControl();
};

class ShiftSpeedControl 
: public simparm::Object,
  public _ShiftSpeedControl,
  public StateMachine::Listener,
  public simparm::Node::Callback
{
    StateMachine &sm;
    simparm::DataChoiceEntry<OutputAmp>& outputAmp;
    simparm::DataChoiceEntry<int>& VS_Speed, HS_Speed;

    void fillADChannel();
    void fillHSSpeed();
    void fillVSSpeed();
    void registerNamedEntries();
  public:
    ShiftSpeedControl(StateMachine &sm, Config& conf);
    ShiftSpeedControl(const ShiftSpeedControl&c);
    ~ShiftSpeedControl();

    void operator()(const simparm::Event&);
    void controlStateChanged(Phase phase, State from, State to);
};

}

#endif
