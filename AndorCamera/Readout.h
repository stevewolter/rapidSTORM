#ifndef ANDORCAMERA_READOUT_H
#define ANDORCAMERA_READOUT_H

#include <AndorCamera/StateMachine.h>
#include <simparm/NumericEntry.hh>
#include <AndorCamera/Config.h>
#include <list>

namespace AndorCamera {                                                    

class Readout 
: public simparm::Object,
  public virtual StateMachine::Listener
{
    simparm::BoolEntry frame_transfer_mode;
  public:
    Readout(const std::string& name, const std::string& desc);
    Readout(const Readout& c);
    void registerNamedEntries(simparm::Node& n)
        { n.push_back( frame_transfer_mode ); }

    template <int State> class Token;
};

class ImageReadout
: public Readout,
  public simparm::Node::Callback,
  public StateMachine::StandardListener<ImageReadout>
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

    void operator()(const simparm::Event&);
    template <int State> class Token;
};

}

#endif
