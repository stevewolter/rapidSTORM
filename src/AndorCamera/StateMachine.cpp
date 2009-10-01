#include "StateMachine.h"
#include "SDK.h"
#include "System.h"
#include <cassert>
#include <algorithm>

namespace AndorCamera {

using namespace States;
using namespace Phases;

StateMachine::StateMachine(int index) 
: status("CameraStatus", "Status of camera " + std::string(1, '1'+index)),
  current_state( Disconnected ),
  camID(index)
{
}

StateMachine::~StateMachine() 
{
    assert( current_state == Disconnected );
    while ( ! listeners.empty() )
        remove_listener( *listeners.front() );
}

void StateMachine::bring_to_state( Listener &l, State from, State to )

{
    l.controlStateChanged( Beginning, from, to );
    State s = from, n;
    while ( (n = next_state_in_direction(s, to)) != s ) 
    {
        l.controlStateChanged( Prepare, s, n );
        l.controlStateChanged( Transition, s, n );
        l.controlStateChanged( Review, s, n );
        s = n;
    }
    l.controlStateChanged( Ending, from, to );
}

void StateMachine::add_listener( Listener &l) 
{
    ost::MutexLock lock(mutex);
    /* No states are possible while the system is acquiring; thus go down first
     * if it is. */
    if ( current_state == Acquiring )
        go_to_state( lower_state(Acquiring) );

    listeners.push_back( ListenerReference(&l) );
    if ( current_state != Disconnected ) 
        bring_to_state(l, Disconnected, current_state); 
}

void StateMachine::passivate_listener( Listener &l) {
    ost::MutexLock lock(mutex);
    Listeners::iterator i = std::find( listeners.begin(), listeners.end(), &l );
    if ( i != listeners.end() ) i->is_active = false;
}

void StateMachine::activate_listener( Listener &l) {
    ost::MutexLock lock(mutex);
    Listeners::iterator i = std::find( listeners.begin(), listeners.end(), &l );
    if ( i != listeners.end() ) i->is_active = true;
}

void StateMachine::remove_listener( Listener &l ) {
    ost::MutexLock lock(mutex);
    /* No states are possible while the system is acquiring; thus go down first
     * if it is. */
    if ( current_state == Acquiring )
        go_to_state( lower_state(Acquiring) );

    listeners.erase( std::find( listeners.begin(), listeners.end(), &l ) );
    if ( current_state != Disconnected ) 
        bring_to_state(l, current_state, Disconnected); 
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

bool in_range( 
    State check,
    State range_from,
    State range_to
)
{
    return (range_from <= check && range_to >= check);
}

void StateMachine::propagate(Phase phase, State from, State to) {
    if ( from < to ) {
        for (Listeners::iterator i = listeners.begin(); i != listeners.end(); i++)
          if ( phase != Transition || i->is_active )
            (*i)->controlStateChanged( phase, from, to );
    } else {
        for (Listeners::iterator i = listeners.end(); i != listeners.begin(); )
        {
            i--;
            if ( phase != Transition || i->is_active )
                (*i)->controlStateChanged( phase, from, to );
        }
    }
}

void StateMachine::go_to_state(State new_state)
 
{
    STATUS("Going from state " << current_state << " to state " << new_state);

    /* First ensure that the current camera is me. */
    System::singleton().selectCamera( camID );

    State old_state = current_state;

    /* Set managed attributes to false where necessary. */
    for (ManagedObjects::iterator i = managed_objects.begin();
                                    i != managed_objects.end(); i++)
        if (     in_range( old_state, i->from, i->to ) &&
               ! in_range( new_state, i->from, i->to ) )
            i->entry = false;

    /* Send AbortAcquisition signal here to make sure it comes first. */
    if ( old_state == Acquiring ) {
        while ( SDK::GetStatus() == SDK::Is_Acquiring ) {
            STATUS("Waiting for acquisition to terminate");
            SDK::WaitForAcquisition();
        }
    }

    /* Send LeavingState signal. */
    STATUS("Leaving state " << old_state << " for " << new_state);
    propagate( Beginning, old_state, new_state );

    /* Send Transition signals. */
    while ( current_state != new_state ) {
        State next_state = 
                    next_state_in_direction( current_state, new_state );

        STATUS("Changing state " << current_state << " to " << next_state);
        propagate( Prepare, current_state, next_state );
        propagate( Transition, current_state, next_state );
        propagate( Review, current_state, next_state );

        current_state = next_state;
        STATUS("Changed state " << current_state << " to " << next_state);
    }

    /* Send EnteringState signals. */
    STATUS("Entering state " << new_state << " from " << old_state);
    propagate( Ending, old_state, new_state );
    STATUS("Entered state " << new_state << " from " << old_state);

    /* Send StartAcquisition signal here to make sure it comes last. */
    if ( new_state == Acquiring ) {
        status = "Acquiring images";
        SDK::StartAcquisition();
    }

    /* Set managed attributes to true where necessary. */
    for (ManagedObjects::iterator i = managed_objects.begin();
                                    i != managed_objects.end(); i++)
        if (   ! in_range( old_state, i->from, i->to ) &&
                 in_range( new_state, i->from, i->to ) )
            i->entry = true;

    STATUS("Finished state transition");

}

State States::higher_state(State state) {
    switch (state) {
        case Disconnected: return Initialized;
        case Initialized: return Acquiring;
        default: return state;
    }
}

State States::lower_state(State state) {
    switch (state) {
        case Initialized: return Disconnected;
        case Acquiring: return Initialized;
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

void StateMachine::add_managed_attribute(
        simparm::Attribute<bool>& attribute,
        State true_from, 
        State true_to )

{
    managed_objects.push_back( Managed( attribute, true_from, true_to ) );
    attribute = in_range( current_state, true_from, true_to );
}

void StateMachine::remove_managed_attribute(simparm::Attribute<bool>& a)

{
    for (ManagedObjects::iterator i = managed_objects.begin();
                                  i!= managed_objects.end(); i++)
        if ( &(i->entry) == &a ) {
            managed_objects.erase( i );
            break;
        }
}

StateMachine::Listener::~Listener() {
    while ( ! listening_to.empty() )
        listening_to.front()->remove_listener( *this );
}

};
