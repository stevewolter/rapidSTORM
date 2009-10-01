#ifndef ANDORCAMERA_INITIALIZATION_H
#define ANDORCAMERA_INITIALIZATION_H

#include <AndorCamera/StateMachine.h>
#include <simparm/FileEntry.hh>
#include <simparm/TriggerEntry.hh>

namespace AndorCamera {                                                    

class _Initialization {
  public:
    /** The directory to read camera ini files from. */
    simparm::FileEntry         configDir;
    /** Triggering this item makes the system connect to the chosen
        *  camera. */
    simparm::TriggerEntry      connect;
    /** Triggering this item makes the system connect to the chosen
        *  camera and initialize the fields below. */
    simparm::TriggerEntry      disconnect;

    _Initialization();
};

class Initialization 
: public simparm::Object,
  public _Initialization,
  public StateMachine::Listener,
  public simparm::Node::Callback
{
    StateMachine &sm;
    void registerNamedEntries();
  public:
    Initialization(StateMachine &sm);
    Initialization(const Initialization&c);
    ~Initialization();

    void operator()(Node &, Cause, Node *);
    void controlStateChanged(Phase phase, State from, State to);
};

}

#endif
