#ifndef ANDORCAMERA_READOUT_H
#define ANDORCAMERA_READOUT_H

#include <AndorCamera/StateMachine.h>
#include <simparm/NumericEntry.hh>
#include <AndorCamera/Config.h>
#include <list>

namespace AndorCamera {                                                    

class Readout 
: public virtual simparm::Node,
  public StateMachine::Listener
{
    simparm::BoolEntry frame_transfer_mode;
  public:
    Readout() 
    : frame_transfer_mode("FrameTransferMode", "Enable frame transfer mode", true)
    { frame_transfer_mode.setUserLevel(simparm::Entry::Expert); push_back(frame_transfer_mode); }
    Readout(const Readout& c) 
    : simparm::Node(c), 
      StateMachine::Listener(),
      frame_transfer_mode(c.frame_transfer_mode)
    { push_back(frame_transfer_mode); }

    void controlStateChanged(Phase phase, State from, State to);
};

class _ImageReadout : public simparm::Object {
  public:
    simparm::UnsignedLongEntry left, top, right, bottom;

    _ImageReadout();
};

class ImageReadout
: public Readout,
  public _ImageReadout,
  public simparm::Node::Callback
{
    StateMachine &sm;
    void registerNamedEntries();

  public:
    ImageReadout(StateMachine &sm);
    ImageReadout(const ImageReadout&c);
    ~ImageReadout();
    ImageReadout* clone() { return new ImageReadout(*this); }

    void operator()(Node &, Cause, Node *);
    void controlStateChanged(Phase phase, State from, State to);
};

}

#endif
