#include "debug.h"

#include "StateMachine.h"
#include "SDK.h"
#include "System.h"
#include <cassert>
#include <algorithm>
#include <dStorm/error_handler.h>

namespace AndorCamera {

using namespace States;

class EmergencyCallback : public dStorm::Runnable {
        StateMachine &sm;
    public:
        EmergencyCallback(StateMachine &sm) : sm(sm) {}
        void run() { 
            DEBUG("Running error handler");
            /* Release error handler to ensure it is never deleted and
             * thus never gives up its state machine control. */
            sm.ensure_at_most( States::Disconnected, 
                               StateMachine::Emergency ).release();
        }
};

struct MutexUnlock {
    ost::Mutex& m;
    MutexUnlock(ost::Mutex& m) :m(m) { m.leaveMutex(); }
    ~MutexUnlock() { m.enterMutex(); }
};

struct StateMachine::InterruptedTransition
: public std::runtime_error
{
    InterruptedTransition() 
        : std::runtime_error("Interrupted camera activity") {}
};

StateMachine::StateMachine(int index) 
: ost::Thread("Camera manager"),
  status("CameraStatus", "Status of camera " + std::string(1, '1'+index)),
  state("NumericState", "Numeric state of cam " + std::string(1, '1'+index)),
  desired_state_changed(locked_state),
  desired_state_reached(locked_state),
  should_shut_down(false),
  currently_served( NULL ),
  current_state( Disconnected ),
  desired_state( Disconnected ),
  emergency_callback( new EmergencyCallback(*this) ),
  camID(index)
{
    dStorm::ErrorHandler::get_current_handler()
        .add_emergency_callback( *emergency_callback );
    state.userLevel = simparm::Object::Debug;
    state.viewable = false;
    Thread::start();
}

StateMachine::~StateMachine() 
{
    should_shut_down = true;
    desired_state_changed.signal();
    Thread::join();
    dStorm::ErrorHandler::get_current_handler()
        .remove_emergency_callback( *emergency_callback );
    assert( current_state == Disconnected );
    while ( ! listeners.empty() )
        remove_listener( *listeners.front() );
}

void StateMachine::push_token( ListenerReference& creator,
                               std::auto_ptr<States::Token> token, 
                               State to )
{
    token->tag = creator.p;
    DEBUG("Adding token " << token->tag << " to " << to);
    ost::MutexLock lock( state_stack_mutex );
    state_stack[to].push_front( token );
}

void StateMachine::raise_state(ListenerReference &l, State to) {
    push_token( l, l.p->raise_state(to), to );
}

void StateMachine::go_to_state( State to )
{
    if ( to < current_state ) {
        while ( current_state != to ) {
            try {
                ost::MutexLock lock( state_stack_mutex );
                while ( ! state_stack[current_state].empty() ) {
                    check_for_interruption( Down );
                    DEBUG("Removing token " << state_stack[current_state].front().tag << " from " << current_state);
                    MutexUnlock unlock(locked_state);
                    state_stack[current_state].pop_front();
                }
                check_for_interruption( Down );
            } catch (const InterruptedTransition&) {
                throw std::logic_error(
                    "Interruption during camera down-going unsupported");
            }
            current_state = lower_state(current_state);
            state = current_state;
        }
    } else if ( current_state < to ) {
        while ( current_state != to )
        {
            State n = higher_state(current_state);
            try {
                for (Listeners::iterator i  = listeners.begin(); 
                                        i != listeners.end(); i++)
                {
                    check_for_interruption( Up );
                    MutexUnlock unlock(locked_state);
                    raise_state( *i, n );
                }
                check_for_interruption( Up );
            } catch (...) {
                ost::MutexLock lock( state_stack_mutex );
                while ( ! state_stack[n].empty() ) {
                    DEBUG("Removing token " << state_stack[n].front().tag << " from " << n);
                    state_stack[n].pop_front();
                }
                throw;
            }
            current_state = n;
            state = current_state;
        }
    }
}

void StateMachine::bring_to_state( ListenerReference& l, State to ) 
{
    State s = Disconnected;
    while ( s != to ) {
        State n = higher_state(s);
        raise_state( l, n );
        s = n;
    }
}
void StateMachine::bring_to_disconnect( Listener& l ) {
    State s = current_state;
    do {
        ost::MutexLock lock( state_stack_mutex );
        StateStack::mapped_type::iterator i = state_stack[s].begin();
        while ( i != state_stack[s].end() ) {
            DEBUG("Iterating over " << s);
            if ( i->tag == &l ) {
                DEBUG("Erasing element for " << i->tag << " at " << s);
                i = state_stack[s].erase(i);
            } else
                ++i;
        }

        if ( s == Disconnected ) break;
        s = lower_state(s);
    } while ( true );
}

void StateMachine::add_listener( Listener &l) 
{
    ost::MutexLock lock(listener_list);
    ost::MutexLock lock2(locked_state);
    DEBUG("Adding listener on state " << current_state);
    /* No states are possible while the system is acquiring; thus go down first
     * if it is. */
    if ( current_state < Acquiring ) {
        listeners.push_back( ListenerReference(&l) );
        if ( current_state != Disconnected ) {
            MutexUnlock unlock( locked_state );
            bring_to_state(listeners.back(), current_state); 
        }
    } else {
        assert( false /* Tried to add listener while acquiring */ );
        throw std::logic_error("Adding listener while acquiring");
    }
}

void StateMachine::passivate_listener( Listener &l) {
    ost::MutexLock lock(listener_list);
    Listeners::iterator i = std::find( listeners.begin(), listeners.end(), &l );
    if ( i != listeners.end() ) (*i)->is_active = false;
}

void StateMachine::activate_listener( Listener &l) {
    ost::MutexLock lock(listener_list);
    Listeners::iterator i = std::find( listeners.begin(), listeners.end(), &l );
    if ( i != listeners.end() ) (*i)->is_active = true;
}

void StateMachine::remove_listener( Listener &l ) {
    ost::MutexLock lock(listener_list);
    ost::MutexLock lock2(locked_state);
    DEBUG("Removing listener on state " << current_state);
    Listeners::iterator i = std::find( listeners.begin(), listeners.end(), &l );
    assert( i != listeners.end() );
    listeners.erase( i );
    MutexUnlock unlock( locked_state );
    bring_to_disconnect(l); 
}

std::auto_ptr<StateMachine::Request>
StateMachine::ensure_precisely(State newState, Priority p)
 
{
    return mkrequest(newState, p, Precisely);
}

std::auto_ptr<StateMachine::Request>
StateMachine::ensure_at_least(State newState, Priority p) 
{
    return mkrequest(newState, p, HigherOK);
}

std::auto_ptr<StateMachine::Request>
StateMachine::ensure_at_most(State newState, Priority p)
{
    return mkrequest(newState, p, LowerOK);
}

State States::higher_state(State state) {
    switch (state) {
        case Disconnected: return Connecting;
        case Connecting: return Connected;
        case Connected: return Readying;
        case Readying: return Ready;
        case Ready: return Acquiring;
        default: return state;
    }
}

State States::lower_state(State state) {
    switch (state) {
        case Connecting: return Disconnected;
        case Connected: return Connecting;
        case Readying: return Connected;
        case Ready: return Readying;
        case Acquiring: return Ready;
        default: return state;
    }
}

#if 0
State States::next_state_in_direction(State state, State dir)
{
    if ( state < dir )
        return higher_state(state);
    else if ( state > dir )
        return lower_state(state);
    else
        return state;
}
#endif

StateMachine::Listener::~Listener() {
    while ( ! listening_to.empty() )
        listening_to.front()->remove_listener( *this );
}

void StateMachine::run() {
    try {
        ost::MutexLock lock(locked_state);
        while ( ! should_shut_down )
        {
            if ( request_is_fulfilled() ) {
                signal_fulfillment();
            } else {
                try {
                    DEBUG("Going to state " << desired_state);
                    go_to_state( desired_state );
                    DEBUG("Went to state " << desired_state << 
                          ", am at state " << current_state);
                } catch (const InterruptedTransition& ) {
                } catch (const Request::Failure& ) {
                }
                signal_fulfillment();
            }
        }
        DEBUG("Finished state machine loop");
        return;
    } catch (const std::exception& e) {
        std::cerr << "Error in driving camera: " << e.what() << std::endl;
    }
    ost::MutexLock lock(locked_state);
    desired_state_reached.signal();
    while ( ! should_shut_down ) {
      try {
        desired_state_changed.wait();
        if ( desired_state == Disconnected ) {
            try { 
                go_to_state( Disconnected );
                signal_fulfillment();
            } catch (const InterruptedTransition& ) {}
        } else {
            ost::MutexLock lock( request_queue );
            DEBUG("Locked request queue in " << __LINE__);
            currently_served->cancel();
        }
      } catch (const std::exception& e) {
        std::cerr << "Error in camera shutdown: " 
                  << e.what() << std::endl;
      }
    }
}

void StateMachine::signal_fulfillment() {
    if ( currently_served != NULL 
        && ! currently_served->has_managing_thread ) 
    {
        DEBUG("Deleting finished request");
        delete currently_served;
        DEBUG("Deleted finished request");
    } else {
        DEBUG("Signalling request is finished");
        desired_state_reached.signal();
        DEBUG("Waiting for next task");
        desired_state_changed.wait();
        DEBUG("Got next task");
    }
}

/** Method assumes locked_state mutex ownership. */
void StateMachine::check_for_interruption(Direction dir) {
    if (  (dir == Up && desired_state < current_state )
       || (dir == Down && desired_state > current_state ) )
       throw InterruptedTransition();
    else if ( currently_served )
        currently_served->check();
}

void StateMachine::wait_or_abort_transition(
    Direction dir, int milliseconds)
{
    ost::MutexLock lock(locked_state);
    check_for_interruption(dir);
    desired_state_changed.timed_wait( milliseconds );
    check_for_interruption( dir );
}

std::auto_ptr<StateMachine::Request>
StateMachine::mkrequest(State s, Priority p, Tolerance t)
{
    DEBUG("Waiting for request queue lock");
    ost::MutexLock lock( request_queue );
    DEBUG("Making request for " << s << " " << p << " " << t );
    std::auto_ptr<Request> me( new Request(*this, s,p,t) );
    if ( currently_served == NULL ) {
        DEBUG("Granting immediately on empty camera");
        currently_served = me.get();
    } else if (should_shut_down && s != Disconnected) {
        throw std::runtime_error("Camera shutdown in progress");
    } else if ( currently_served->priority > p ) 
    {
        DEBUG("Denied because more important request is on camera, priority " << currently_served->priority);
        throw Request::Failure(currently_served->priority);
    } else if ( currently_served->priority == p && p != User ) {
        DEBUG("Inserted request into wait queue");
        me->insert( waiting.end() );
        me->wait_for_activation();
    } else {
        DEBUG("Granting, Request overrides lower-priority requests");
        me->insert( waiting.begin() );
        currently_served->cancel( p == Emergency );
        while ( ! waiting.empty() && waiting.back() != me.get() )
            waiting.front()->cancel();
        assert( p != Emergency || me.get() == currently_served );
        DEBUG("Currently active is " << currently_served << " with me "
              << me.get());
        me->wait_for_activation();
        DEBUG("Override got active");
    }

    ost::MutexLock lock2( locked_state );
    me->serve();
    return me;
}

void StateMachine::manage( std::auto_ptr<Request> r ) 
{
    r->has_managing_thread = false;
    ost::MutexLock lock(request_queue);
    DEBUG("Locked request queue in " << __LINE__);
    ost::MutexLock lock2(locked_state);
    if ( r.get() == currently_served && request_is_fulfilled() ) {
        DEBUG("Deleting already fulfilled request");
        r.reset( NULL );
    } else {
        r->check();
        DEBUG("Releasing request pointer; state machine will care");
        r.release();
    }
}
bool StateMachine::request_is_fulfilled() {
    bool f=( (tolerance == Precisely && current_state == desired_state)
           ||(tolerance == LowerOK   && current_state <= desired_state)
           ||(tolerance == HigherOK  && current_state >= desired_state));
    DEBUG("Fulfillment " << f << " at " << tolerance << " " << current_state << " " << desired_state);
    return f;
}

StateMachine::Request::Request(
    StateMachine& sm, State state, Priority p, Tolerance t) 
: was_pushed_off(false), should_be_served(true),
  has_managing_thread(true),
  list_position( sm.waiting.end() ),
  got_active( sm.request_queue ),
  sm(sm),
  state(state), priority(p), tolerance(t)
{
}

/** Assumes ownership of state mutex. */
void StateMachine::Request::serve()
{
    sm.desired_state = state;
    sm.tolerance = tolerance;
    sm.desired_state_changed.signal();
}

void StateMachine::Request::cancel(bool emergency) 
{
    if ( !has_managing_thread )
        delete this;
    else if (emergency && sm.currently_served == this) {
        was_pushed_off = true;
        remove_from_front_of_request_queue();
    } else {
        should_be_served = false;
        remove_from_wait_queue();
    }
}

void StateMachine::Request::insert( std::list<Request*>::iterator where ) 
{
    list_position = sm.waiting.insert( where, this );
}

void StateMachine::Request::remove_from_wait_queue() {
    if ( list_position != sm.waiting.end() ) {
        sm.waiting.erase( list_position );
        list_position = sm.waiting.end();
    }
}
void StateMachine::Request::remove_from_front_of_request_queue()
{
    DEBUG("Removing request from queue");
    if ( sm.waiting.empty() ) {
        DEBUG("No waiting jobs, going to " << sm.should_shut_down);
        ost::MutexLock lock( sm.locked_state );
        sm.desired_state = 
            (sm.should_shut_down) ? Disconnected : Connected;
        sm.tolerance = LowerOK;
        sm.currently_served = NULL;
        sm.desired_state_changed.signal();
    } else {
        sm.currently_served = sm.waiting.front();
        sm.waiting.pop_front();
        sm.currently_served->activate();
    }
    DEBUG("Removed request");
}

void StateMachine::Request::activate() {
    if ( has_managing_thread ) {
        sm.currently_served->got_active.signal();
    } else {
        ost::MutexLock lock( sm.locked_state );
        if ( sm.request_is_fulfilled() )
            delete this;
        else
            serve();
    }
}

StateMachine::Request::~Request() {
    DEBUG("Destructing request, pushed off " << was_pushed_off);
    if ( !was_pushed_off ) {
        ost::MutexLock lock( sm.request_queue );
        DEBUG("Locked request queue in " << __LINE__);
        if ( sm.currently_served == this )
            remove_from_front_of_request_queue();
        else 
            remove_from_wait_queue();
    }
}

void StateMachine::Request::wait_for_fulfillment() {
    ost::MutexLock lock( sm.locked_state );
    check();
    while ( !sm.request_is_fulfilled() ) {
        DEBUG("Entering fulfillment wait");
        sm.desired_state_reached.wait();
        DEBUG("Exited fulfillment wait");
        check();
        DEBUG("Checked fulfillment status, no errors");
    }
}

void StateMachine::Request::check() {
    if ( was_pushed_off ) {
        /* Instead of throwing an error here that might trigger
         * more bugs, just loop infinitely. */
        while ( true ) ;
        throw Request::Failure(Emergency);
    }

    if ( !should_be_served ) {
        /* If we should give up the camera, some better reason must
         * be in the queue. */
        assert( ! sm.waiting.empty() );
        assert( sm.waiting.front() != NULL );
        throw Request::Failure(sm.waiting.front()->priority);
    }
}

void StateMachine::Request::wait_for_activation() {
    while ( sm.currently_served != this ) {
        check();
        got_active.wait();
    }
}

StateMachine::Request::Failure::Failure(Priority overridden_with)
: std::runtime_error(
    (overridden_with == Emergency) ? "Interrupted by emergency shutdown"
  : (overridden_with == OtherCameraActive) ? 
        "A different camera is currently active"
  : (overridden_with == User ) ?
        "User overruled automatic camera control"
  : (overridden_with == Auto ) ?
        "Automatic process interrupting idle job"
  : "Unknown failure cause"
  ),
  was_overriden_with(overridden_with)
{
    
}

};
