#ifndef ANDORCAMERA_STATEMACHINE_H
#define ANDORCAMERA_STATEMACHINE_H

#include <simparm/Attribute.hh>
#include <simparm/Entry.hh>
#include <simparm/NumericEntry.hh>
#include <map>
#include <dStorm/helpers/thread.h>
#include <boost/ptr_container/ptr_list.hpp>

#include <iostream>

namespace AndorCamera {

class StateMachine;

/** The possible states a camera might be in. */
namespace States { 
    enum State {
        /** Starting state, no connection to camera. */
        Disconnected,    
        /** Making connection to camera. */
        Connecting, 
        /** Connection to camera established. This is our basic working
         *  state, and finished acquisitions let the camera return to
         *  this state. */
        Connected, 
        /** Bringing camera to acquisition readiness */
        Readying, 
        /** Camera is ready for acquiring */
        Ready,
        /** Camera is acquiring */
        Acquiring 
    };

    extern State higher_state(State state);
    extern State lower_state(State state);
    extern State next_state_in_direction(State state, State dir);

    struct Token {
        void *tag;
        Token() {}
        virtual ~Token() {}
    };
};
using States::State;

class StateMachine {
  public:

    /** Base class for objects wishing to be informed about changes
     *  of camera state. */
    class Listener;
    template <typename Type> class StandardListener;

    StateMachine(int for_camera_index);
    StateMachine(const StateMachine &);
    ~StateMachine();

    void add_listener(Listener& l);
    void remove_listener(Listener& l);

    /** If a listener is passivated, it will not receive Transition events
     *  anymore, thus precluding active changes to the camera. */
    void passivate_listener(Listener &l);
    void activate_listener(Listener &l);

    /** Reach exactly the given state.
      *  \param newState State to reach */
    void ensure_precisely(State newState);
    /** Reach the given state or a higher one.
      *  \param newState State to reach */
    void ensure_at_least(State newState);
    /** Reach the given state or a lower one.
      *  \param newState State to reach */
    void ensure_at_most(State newState);

    simparm::StringEntry status;
    simparm::UnsignedLongEntry state;

    int get_cam_id() const { return camID; }

  private:
    ost::Mutex mutex;
    State current_state;

    /** StateStack is implemented as a map of one list
     *  per state. */
    typedef std::map< State, boost::ptr_list<States::Token> > StateStack;
    StateStack state_stack;

    struct ListenerReference;
    void push_token( ListenerReference& creator,
                     std::auto_ptr<States::Token> token, State to );
    void go_to_state(State new_state);
    void bring_to_state(ListenerReference &l, State to);
    void bring_to_disconnect(Listener &l);
    void raise_state(ListenerReference &l, State to);
    int camID;

    struct ListenerReference {
        Listener *p;

        ListenerReference(Listener *p) : p(p) {}
        Listener* operator->() { return p; }
        Listener& operator*() { return *p; }
        bool operator==(const ListenerReference& o) { return p == o.p; }
        bool operator!=(const ListenerReference& o) { return p != o.p; }
    };
    typedef std::list< ListenerReference > Listeners;
    Listeners listeners;
};

class StateMachine::Listener {
    friend class StateMachine;
    std::list<StateMachine*> listening_to;

  protected:
    /** The listener is currently active */
    bool is_active;

    Listener() : is_active(true) {}

    /** The destructor unregisters the listener from the Camera, if
     *  the user did not.*/
    virtual ~Listener();

    /** Raise the state of that listener. */
    virtual std::auto_ptr<States::Token> raise_state(States::State to) = 0;
};

template <typename ReachedStateProducer>
class StateMachine::StandardListener 
: public virtual StateMachine::Listener {
  private:
    ReachedStateProducer& p;

  protected:
    StandardListener(ReachedStateProducer& p) : p(p) {}

    std::auto_ptr<States::Token> raise_state(States::State to);
};

}

#endif
