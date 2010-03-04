#ifndef ANDORCAMERA_CAMERA_H
#define ANDORCAMERA_CAMERA_H

#include <memory>
#include <simparm/Object.hh>
#include <simparm/ChoiceEntry.hh>
#include <dStorm/helpers/thread.h>
#include <queue>
#include <AndorCamera/StateMachine.h>
#include <AndorCamera/CameraReference.h>

namespace AndorCamera {

class Config;
class Initialization;
class Temperature;
class Gain;
class Readout;
class AcquisitionModeControl;
class Triggering;
class ShiftSpeedControl;
class ShutterControl;
class AcquisitionSwitch;
class Acquisition;

class Camera : public simparm::Object {
  private:
    std::auto_ptr<StateMachine> _state_machine;

    std::auto_ptr<Config> _config;

    std::auto_ptr<Initialization> _initialization;
    std::auto_ptr<Temperature> _temperature;
    std::auto_ptr<Gain> _gain;
    std::auto_ptr<Readout> _readout;
    std::auto_ptr<AcquisitionModeControl> _acquisitionMode;
    std::auto_ptr<Triggering> _triggering;
    std::auto_ptr<ShiftSpeedControl> _shift_speed_control;
    std::auto_ptr<ShutterControl> _shutter_control;
    std::auto_ptr<AcquisitionSwitch> _acquisition_switch;

  public:
    Camera(int index);
    ~Camera();
    Camera(const Camera &c);
    Camera* clone() const;
    Camera& operator=(const Camera &c);

    ost::Mutex mutex;

    StateMachine& state_machine() { return *_state_machine; }
    Config& config() { return *_config; }
    Initialization& initialization() { return *_initialization; }
    Temperature& temperature() { return *_temperature; }
    Gain& gain() { return *_gain; }
    Readout& readout() { return *_readout; }
    AcquisitionModeControl& acquisitionMode() 
        { return *_acquisitionMode; }
    Triggering& triggering() { return *_triggering; }
    ShiftSpeedControl& shift_speed_control()
        { return *_shift_speed_control; }
    ShutterControl& shutter_control() { return *_shutter_control; }

    class ExclusiveAccessor {
        friend class Camera;
        CameraReference camera;
        typedef std::list< std::pair<StateMachine::Listener*,StateMachine::Listener*> >
            Replacements;
        Replacements replace;

        void _got_access();

      public:
        ExclusiveAccessor(const CameraReference& forCamera) : camera(forCamera) {}

        void replace_on_access( StateMachine::Listener& to_replace,
                                StateMachine::Listener& replace_with );
        /** Request exclusive access to the given camera. Once the access is granted,
         *  the got_access() method will be called. */
        void request_access();
        /** This method is called once the Accessor got exclusive access to the given
         *  camera. This exclusive access will persist until explicitely ended with
         *  forfeit_access() or until the ExclusiveAccessor class is destructed. */
        virtual void got_access() = 0;
        /** Another accessor is knocking, that is, requesting access to the camera. */
        virtual void other_accessor_is_knocking() {}
        /** Give up the exclusive access to the camera. This method will push 
         *  waiting accessors onto the camera immediately and notify them,
         *  possibly throwing exceptions through this method. */
        virtual void forfeit_access();
        /** The destructor will automatically release exclusive camera access.
         *  This method will push waiting accessors onto the camera immediately 
         *  and notify them, possibly throwing exceptions through this method. */
        virtual ~ExclusiveAccessor();
    };

  private:
    friend class ExclusiveAccessor;
    ExclusiveAccessor* current_accessor;
    std::queue<ExclusiveAccessor*> waiting_accessors;
};

};

#endif
