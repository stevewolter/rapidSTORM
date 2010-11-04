//#define VERBOSE
#include "debug.h"
#include "Camera.h"
#include "StateMachine.h"
#include "Acquisition.h"
#include "Initialization.h"
#include "Temperature.h"
#include "Readout.h"
#include "AcquisitionMode.h"
#include "Triggering.h"
#include "ShiftSpeedControl.h"
#include "ShutterControl.h"
#include "AcquisitionSwitch.h"
#include "Gain.h"
#include "Config.h"
#include <simparm/ChoiceEntry_Impl.hh>
#include <algorithm>

namespace AndorCamera {

Camera::Camera(int id) 
:   simparm::Object("Camera", "Camera configuration"),
    _state_machine(new StateMachine(id)),
    _config(new Config()),
    _initialization(new Initialization(*_state_machine)),
    _temperature(new Temperature(*_state_machine, *_config)),
    _gain(new Gain(*_state_machine, *_config)),
    _readout(new ImageReadout(*_state_machine)),
    _acquisitionMode(new AcquisitionModeControl(*_state_machine, *_config)),
    _triggering(new Triggering(*_state_machine)),
    _shift_speed_control(new ShiftSpeedControl(*_state_machine, *_config)),
    _shutter_control(new ShutterControl(*_state_machine)),
    _acquisition_switch(new AcquisitionSwitch(*_state_machine))
{
    _state_machine->add_listener(*_initialization);
    _state_machine->add_listener(*_temperature);
    _state_machine->add_listener(*_readout);
    _state_machine->add_listener(*_acquisitionMode);
    _state_machine->add_listener(*_triggering);
    _state_machine->add_listener(*_shift_speed_control);
    _state_machine->add_listener(*_shutter_control);
    _state_machine->add_listener(*_gain);
    _state_machine->add_listener(*_acquisition_switch);

    push_back(*_initialization);
    push_back(*_temperature);
    push_back(*_readout);
    push_back(*_acquisitionMode);
    push_back(*_triggering);
    push_back(*_shift_speed_control);
    push_back(*_shutter_control);
    push_back(*_gain);
}

Camera* Camera::clone() const {
    assert( false /* AndorCamera::Camera::clone called */ );
    throw std::runtime_error("Camera object cloning not implemented.");
}

Camera::~Camera() 
{
    DEBUG("Shutting camera down");
    std::auto_ptr<StateMachine::Request> r =
        state_machine().ensure_at_most(
            States::Disconnected, StateMachine::Idle);
    r->wait_for_fulfillment();
    DEBUG("Shutdown complete, destructing");
}

Camera::ExclusiveAccessor::ExclusiveAccessor
    (const CameraReference& forCamera)
 : camera(forCamera) , 
   gained_access(camera->mutex ),
   exploited_access(false)
 {}
void Camera::ExclusiveAccessor::replace_on_access( StateMachine::Listener& to_replace,
    StateMachine::Listener& replace_with )
{
    replace.push_back( std::make_pair(&to_replace, &replace_with) );
    assert ( ! exploited_access ); 
}

void Camera::ExclusiveAccessor::request_access() 
{
    ost::MutexLock lock( camera->mutex );
    DEBUG("Putting accessor " << this << " into queue");
    camera->waiting_accessors.push_back( this );
}

void Camera::ExclusiveAccessor::wait_for_access() {
    ost::MutexLock lock( camera->mutex );
    DEBUG("Accessor " << this << " is waiting for access");
    while ( camera->waiting_accessors.front() != this )
        gained_access.wait();
    DEBUG("Accessor " << this << " gained for access");
    for (Replacements::iterator i = replace.begin(); i != replace.end(); i++) {
        camera->state_machine().passivate_listener( *i->first );
        camera->state_machine().add_listener( *i->second );
    }
    exploited_access = true;
}

void Camera::ExclusiveAccessor::forfeit_access() {
    DEBUG(this << " forfeits access to camera");
    ost::MutexLock lock( camera->mutex );
    DEBUG("Got cam mutex");
    if ( camera->waiting_accessors.front() == this ) {
        if ( exploited_access ) {
            for (Replacements::iterator i = replace.begin(); i != replace.end(); i++) {
                camera->state_machine().remove_listener( *i->second );
                camera->state_machine().activate_listener( *i->first );
            }
            exploited_access = false;
        }

        camera->waiting_accessors.pop_front();
        if ( ! camera->waiting_accessors.empty() ) {
            DEBUG("Signalling " << &camera->waiting_accessors.front() << " that it has gained access");
            camera->waiting_accessors.front()->gained_access.signal();
        }
    } else {
        std::list<ExclusiveAccessor*>& w = camera->waiting_accessors;
        std::list<ExclusiveAccessor*>::iterator me = std::find( w.begin(), w.end(), this );
        if ( me != w.end() )
            w.erase( me );
    }
}

Camera::ExclusiveAccessor::~ExclusiveAccessor() {
    forfeit_access();
}

}
