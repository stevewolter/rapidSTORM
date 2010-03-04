#include "StateMachine.h"
#include "SDK.h"
#include "System.h"
#include <cassert>
#include <algorithm>

namespace AndorCamera {

using namespace States;

StateMachine::StateMachine(int index) 
: status("CameraStatus", "Status of camera " + std::string(1, '1'+index)),
  state("NumericState", "Numeric state of cam " + std::string(1, '1'+index)),
  current_state( Disconnected ),
  camID(index)
{
    state.userLevel = simparm::Entry::Debug;
    state.viewable = false;
}

StateMachine::~StateMachine() 
{
    assert( current_state == Disconnected );
    while ( ! listeners.empty() )
        remove_listener( *listeners.front() );
}

void StateMachine::push_token( ListenerReference& creator,
                               std::auto_ptr<States::Token> token, 
                               State to )
{
    token->tag = creator.p;
    state_stack[to].push_front( token );
}

void StateMachine::raise_state(ListenerReference &l, State to) {
    push_token( l, l.p->raise_state(to), to );
}

void StateMachine::go_to_state( State to )
{
    if ( to < current_state ) {
        while ( current_state != to ) {
            while ( ! state_stack[current_state].empty() )
                state_stack[current_state].pop_front();
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
                    raise_state( *i, n );
                }
            } catch (...) {
                while ( ! state_stack[n].empty() )
                    state_stack[n].pop_front();
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
        StateStack::mapped_type::iterator i = state_stack[s].begin();
        while ( i != state_stack[s].end() ) {
            if ( i->tag == &l )
                i = state_stack[s].erase(i);
            else
                ++i;
        }

        if ( s == Disconnected ) break;
        s = lower_state(s);
    } while ( true );
}

void StateMachine::add_listener( Listener &l) 
{
    ost::MutexLock lock(mutex);
    /* No states are possible while the system is acquiring; thus go down first
     * if it is. */
    State saved_state = current_state;
    if ( current_state == Acquiring )
        go_to_state( lower_state(Acquiring) );

    listeners.push_back( ListenerReference(&l) );
    if ( current_state != Disconnected ) 
        bring_to_state(listeners.back(), current_state); 

    go_to_state( saved_state );
}

void StateMachine::passivate_listener( Listener &l) {
    ost::MutexLock lock(mutex);
    Listeners::iterator i = std::find( listeners.begin(), listeners.end(), &l );
    if ( i != listeners.end() ) (*i)->is_active = false;
}

void StateMachine::activate_listener( Listener &l) {
    ost::MutexLock lock(mutex);
    Listeners::iterator i = std::find( listeners.begin(), listeners.end(), &l );
    if ( i != listeners.end() ) (*i)->is_active = true;
}

void StateMachine::remove_listener( Listener &l ) {
    ost::MutexLock lock(mutex);
    /* No states are possible while the system is acquiring; thus go down first
     * if it is. */
    State saved_state = current_state;
    if ( current_state == Acquiring )
        go_to_state( lower_state(Acquiring) );

    listeners.erase( std::find( listeners.begin(), listeners.end(), &l ) );
    bring_to_disconnect(l); 
    go_to_state( saved_state );
}

void StateMachine::ensure_precisely(State newState)
 
{
    ost::MutexLock lock(mutex);
    if ( current_state != newState )
        go_to_state( newState );
}

void StateMachine::ensure_at_least(State newState) 
 
{
    ost::MutexLock lock(mutex);
    if ( current_state < newState )
        go_to_state( newState );
}

void StateMachine::ensure_at_most(State newState)
 
{
    ost::MutexLock lock(mutex);
    if ( current_state > newState )
        go_to_state( newState );
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

State States::next_state_in_direction(State state, State dir)

{
    if ( state < dir )
        return higher_state(state);
    else if ( state > dir )
        return lower_state(state);
    else
        return state;
}

StateMachine::Listener::~Listener() {
    while ( ! listening_to.empty() )
        listening_to.front()->remove_listener( *this );
}

};
