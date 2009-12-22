#ifndef ANDORCAMERA_READOUT_H
#define ANDORCAMERA_READOUT_H

#include <AndorCamera/StateMachine.h>
#include <simparm/NumericEntry.hh>
#include <AndorCamera/Config.h>
#include <list>

namespace AndorCamera {                                                    

class Readout 
: public simparm::Object,
  public StateMachine::Listener
{
    simparm::BoolEntry frame_transfer_mode;
  public:
    Readout(const std::string& name, const std::string& desc) 
        : simparm::Object(name, desc),
          frame_transfer_mode("FrameTransferMode", 
                              "Enable frame transfer mode", true)
        { frame_transfer_mode.setUserLevel(simparm::Entry::Expert); }
    Readout(const Readout& c) 
        : simparm::Object(c), StateMachine::Listener(),
          frame_transfer_mode(c.frame_transfer_mode) {}
    void registerNamedEntries(simparm::Node& n)
        { n.push_back( frame_transfer_mode ); }

    void controlStateChanged(Phase phase, State from, State to);
};

class ImageReadout
: public Readout,
  public simparm::Node::Callback
{
  public:
    simparm::UnsignedLongEntry left, top, right, bottom;

  private:
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
