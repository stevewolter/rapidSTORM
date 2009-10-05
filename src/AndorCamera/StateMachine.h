#ifndef ANDORCAMERA_STATEMACHINE_H
#define ANDORCAMERA_STATEMACHINE_H

#include <simparm/Attribute.hh>
#include <simparm/Entry.hh>
#include <list>
#include <cc++/thread.h>

namespace AndorCamera {

/** The possible states a camera might be in. */
namespace States { 
    enum State {
        /** Starting state, no connection to camera. */
        Disconnected,    
        /** Connected to camera, but cooler is still off. */
        Initialized, 
        /** Camera is acquiring */
        Acquiring 
    };

    extern State higher_state(State state);
    extern State lower_state(State state);
    extern State next_state_in_direction(State state, State dir);
};
using States::State;

namespace Phases {
    /** These values distinguish between the phases of state
        *  transition - leaving the old state, transition 
        *  and entering the new state.
        *  Cleanup of the old state should be done
        *  in the Prepare phase, activities and waiting in the 
        *  Transition phase (like cooling the camera), and things to
        *  be set and initialized in the new state in the Review
        *  phase. Beginning and Ending are called exactly
        *  once atathe beginning and the end of the transition
        *  with the final states as arguments. */
    enum Phase { Beginning, Prepare, Transition, Review, Ending };
};
using Phases::Phase;

class StateMachine {
  public:

    /** Base class for objects wishing to be informed about changes
     *  of camera state. */
    class Listener;

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

    /** The managed attribute given here will be set to true when the
     *  state is set to true_state and false otherwise. */
    void add_managed_attribute(
        simparm::Attribute<bool>& attribute,
        State true_state )
        { add_managed_attribute( attribute, true_state, true_state ); }
    /** The managed attribute given here will be set to true while the
     *  state is in the inclusive range [true_from true_to] and set to
     *  false otherwise. */
    void add_managed_attribute(
        simparm::Attribute<bool>& attribute,
        State true_from, State true_to );

    void remove_managed_attribute( simparm::Attribute<bool>& attribute );

    simparm::StringEntry status;

  private:
    ost::Mutex mutex;
    State current_state;
    void go_to_state(State new_state);
    void bring_to_state(Listener &l, State from, State to);
    int camID;

    struct Managed { 
        simparm::Attribute<bool>& entry;
        State from, to;
        Managed(simparm::Attribute<bool>& e, State f, State t)
            : entry(e), from(f), to(t) {}
    };
    typedef std::list< Managed > ManagedObjects;
    ManagedObjects managed_objects;

    struct ListenerReference {
        Listener *p;
        bool is_active;

        ListenerReference(Listener *p) : p(p), is_active(true) {}
        Listener* operator->() { return p; }
        Listener& operator*() { return *p; }
        bool operator==(const ListenerReference& o) { return p == o.p; }
        bool operator!=(const ListenerReference& o) { return p != o.p; }
    };
    typedef std::list< ListenerReference > Listeners;
    Listeners listeners;

    void propagate(Phase phase, State from, State to);
};

class StateMachine::Listener {
  private:
    friend class StateMachine;
    std::list<StateMachine*> listening_to;

  protected:
    Listener() {}
    /** The destructor unregisters the listener from the Camera, if
     *  the user did not.*/
    virtual ~Listener();

    /** This function is called every time the state of Control
        *  changes. It is called for every state transition; thus,
        *  if the Control class goes from Disconnected to Acquiring,
        *  newState is called eleven times - Beginning:Disconnected,
        *  Prepare:Initialized, Transition:Initialized, Review:Initialized,
        *  Prepare:Cooled, Transition:Cooled, Review:Cooled,
        *  Prepare:Acquiring, Transition:Acquiring, Review:Acquiring,
        *  Ending:Acquiring.
        *  
        *  \param phase    The phase the transition is in.
        *  \param from     The state the control is (or was) in.
        *  \param to       The state the control is going to.
        **/
    virtual void controlStateChanged(Phase phase, State from, State to) 
 = 0;
};

}

#endif
