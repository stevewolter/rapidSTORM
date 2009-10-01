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
#include "Gain.h"
#include "Config.h"
#include <simparm/ChoiceEntry_Impl.hh>

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
    current_accessor(NULL)
{
    _state_machine->add_listener(*_initialization);
    _state_machine->add_listener(*_temperature);
    _state_machine->add_listener(*_readout);
    _state_machine->add_listener(*_acquisitionMode);
    _state_machine->add_listener(*_triggering);
    _state_machine->add_listener(*_shift_speed_control);
    _state_machine->add_listener(*_shutter_control);
    _state_machine->add_listener(*_gain);

    push_back(*_initialization);
    push_back(*_temperature);
    push_back(*_readout);
    push_back(*_acquisitionMode);
    push_back(*_triggering);
    push_back(*_shift_speed_control);
    push_back(*_shutter_control);
    push_back(*_gain);

    state_machine().add_managed_attribute( 
        System::singleton().
            get_camera_chooser().editable, States::Disconnected);
}

Camera* Camera::clone() const {
    throw std::runtime_error("Camera object cloning not implemented.");
}

Camera::~Camera() 
{
    STATUS("Shutting camera down");
    state_machine().ensure_at_most(States::Disconnected);
    STATUS("Shutdown complete, destructing");
}

void Camera::ExclusiveAccessor::replace_on_access( StateMachine::Listener& to_replace,
    StateMachine::Listener& replace_with )
{
    replace.push_back( std::make_pair(&to_replace, &replace_with) );
}

void Camera::ExclusiveAccessor::_got_access() {
    camera->current_accessor = this;
    for (Replacements::iterator i = replace.begin(); i != replace.end(); i++) {
        camera->state_machine().passivate_listener( *i->first );
        camera->state_machine().add_listener( *i->second );
    }
        
    got_access();
}

void Camera::ExclusiveAccessor::request_access() 
{
    ost::MutexLock lock( camera->mutex );
    if ( camera->current_accessor == NULL ) {
        STATUS("Accessor gained camera control immediately.");
        _got_access();
    } else {
        STATUS("Putting accessor into queue.");
        camera->waiting_accessors.push( this );
    }
}

void Camera::ExclusiveAccessor::forfeit_access() {
    STATUS(this << " forfeits access to camera");
    ost::MutexLock lock( camera->mutex );
    LOCKING("Got cam mutex");
    if ( camera->current_accessor == this ) {
        for (Replacements::iterator i = replace.begin(); i != replace.end(); i++) {
            camera->state_machine().remove_listener( *i->second );
            camera->state_machine().activate_listener( *i->first );
        }

        if ( ! camera->waiting_accessors.empty() ) {
            ExclusiveAccessor *next_in_queue = camera->waiting_accessors.front();
            STATUS(this << " has successor " << next_in_queue);
            camera->waiting_accessors.pop();
            next_in_queue->_got_access();
            STATUS(this << " successfully activated successor");
        } else {
            STATUS(this << " has no successor");
            camera->current_accessor = NULL;
        }
    }
}

Camera::ExclusiveAccessor::~ExclusiveAccessor() {
    forfeit_access();
}

}
