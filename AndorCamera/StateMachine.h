#ifndef ANDORCAMERA_STATEMACHINE_H
#define ANDORCAMERA_STATEMACHINE_H

#include <simparm/Attribute.hh>
#include <simparm/Entry.hh>
#include <simparm/NumericEntry.hh>
#include <map>
#include <dStorm/helpers/thread.h>
#include <boost/ptr_container/ptr_list.hpp>

#include <iostream>
#include <semaphore.h>

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

    struct Token {
        void *tag;
        Token() {}
        virtual ~Token() {}
    };
};
using States::State;

class StateMachine 
: private ost::Thread {
  public:

    /** Base class for objects wishing to be informed about changes
     *  of camera state. */
    class Listener;
    template <typename Type> class StandardListener;
    class Request;

    StateMachine(int for_camera_index);
    StateMachine(const StateMachine &);
    ~StateMachine();

    void add_listener(Listener& l);
    void remove_listener(Listener& l);

    /** If a listener is passivated, it will not receive Transition events
     *  anymore, thus precluding active changes to the camera. */
    void passivate_listener(Listener &l);
    void activate_listener(Listener &l);

    enum Priority {
        Idle, Auto, User, OtherCameraActive, Emergency
    };
    enum Direction { Up, Down };
    enum Tolerance { Precisely, LowerOK, HigherOK };

    /** Reach exactly the given state.
      *  \param newState State to reach */
    std::auto_ptr<Request>
        ensure_precisely(State newState, Priority priority );
    /** Reach the given state or a higher one.
      *  \param newState State to reach */
    std::auto_ptr<Request>
        ensure_at_least(State newState, Priority priority );
    /** Reach the given state or a lower one.
      *  \param newState State to reach */
    std::auto_ptr<Request>
        ensure_at_most(State newState, Priority priority);

    void manage( std::auto_ptr<Request> );

    simparm::StringEntry status;
    simparm::UnsignedLongEntry state;

    int get_cam_id() const { return camID; }

    void wait_or_abort_transition(Direction, int milliseconds);
    
  private:
    ost::Mutex locked_state, request_queue, listener_list, state_stack_mutex;
    ost::Condition desired_state_changed, desired_state_reached;
    bool should_shut_down;
    Request *currently_served;
    std::list<Request*> waiting;
    State current_state;
    State desired_state;
    Tolerance tolerance;

    std::auto_ptr<Request> mkrequest(State, Priority, Tolerance);

    /** StateStack is implemented as a map of one list
     *  per state. */
    typedef std::map< State, boost::ptr_list<States::Token> > StateStack;
    StateStack state_stack;

    std::auto_ptr<dStorm::Runnable> emergency_callback;

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

    void run();
    void check_for_interruption(Direction);
    bool request_is_fulfilled();
    void signal_fulfillment();

    class InterruptedTransition;
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

class StateMachine::Request {
    friend class StateMachine;
    bool was_pushed_off, should_be_served, has_managing_thread;
    std::list<Request*>::iterator list_position;
    ost::Condition got_active;
    StateMachine &sm;

    States::State state;
    Priority priority;
    Tolerance tolerance;

    Request(StateMachine& sm,
            State state, Priority priority, Tolerance tolerance);
    void serve();
    void cancel( bool in_emergency = false );
    void insert( std::list<Request*>::iterator where );

    void remove_from_front_of_request_queue();
    void remove_from_wait_queue();

    void wait_for_activation();
    void activate();
    
  public:
    ~Request();

    void wait_for_fulfillment();
    void check();

    class Failure;
};

struct StateMachine::Request::Failure : public std::runtime_error {
    Priority was_overriden_with;
    Failure(Priority overriden_with);
};

}

#endif
