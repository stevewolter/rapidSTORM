#include "debug.h"
#include "Temperature.h"
#include "SDK.h"
#include <string.h>
#include "Config.h"
#include "TemperatureMonitor.h"
#include "System.h"

#include "StateMachine_impl.h"
#include <simparm/EntryManipulators.hh>
#include <dStorm/error_handler.h>

using namespace simparm;
using namespace SDK;

namespace AndorCamera {

using namespace States;

template <typename Type>
struct Setter {
    Type &var;
    Type orig_value;
    Setter(Type& var, Type value) : var(var), orig_value(var)
        { var = value; }
    ~Setter() { DEBUG("Resetting"); var = orig_value; }
};

_Temperature::_Temperature() :
  requiredTemperature("TargetTemperature",
                      "Required CCD temperature for acquisition", -65),
  realTemperature("ActualTemperature","Actual CCD temperature value"),
  doCool("Cooling", "Cool CCD", false)
{
    doCool.setUserLevel(Entry::Beginner);
    doCool.viewable = false;

    requiredTemperature.setHelp("Cool the camera to this value before "
                                "starting acquisitions.");
    requiredTemperature.setUserLevel(Entry::Beginner);
    requiredTemperature.setMax(30);
    requiredTemperature.setMin(-100);

    realTemperature.setHelp("This is the actual CCD temperature.");
    realTemperature.setUserLevel(Entry::Beginner);
    realTemperature.setEditable(false);
    realTemperature.viewable = false;
}

Temperature::Temperature(StateMachine& sm, Config &conf)
: Object("Temperature", "Temperature"),
  StateMachine::StandardListener<Temperature>(*this),
  Node::Callback(simparm::Event::ValueChanged),
  sm(sm),
  targetTemperature(conf.targetTemperature),
  expect_doCool_change(false), am_cooling(false)
{
    registerNamedEntries();
}
  
Temperature::Temperature(const Temperature&c)
: Object(c),
  _Temperature(c),
  StateMachine::StandardListener<Temperature>(*this),
  Node::Callback(simparm::Event::ValueChanged),
  sm(c.sm),
  targetTemperature(c.targetTemperature),
  expect_doCool_change(c.expect_doCool_change),
  am_cooling(c.am_cooling)
{
    registerNamedEntries();
}

Temperature::~Temperature() 
{
}

void Temperature::registerNamedEntries() 
{
    receive_changes_from( doCool.value );
    receive_changes_from( targetTemperature.value );

    push_back( targetTemperature );
    push_back( requiredTemperature );
    push_back( realTemperature );
    push_back( doCool );
}

void Temperature::operator()(const simparm::Event& e)
{
    if ( &e.source == &doCool.value )
    {
        static ost::Mutex event_mutex;
        ost::MutexLock lock( event_mutex );
        DEBUG("Checking cooler value " << doCool() << " " << am_cooling);
        if ( doCool() && !am_cooling ) {
            if ( ! expect_doCool_change ) {
                DEBUG("Requesting state machine to be at least "
                      "connected");
                sm.ensure_at_least(States::Connected, StateMachine::User)
                    ->wait_for_fulfillment();
                DEBUG("Request fulfilled");
            }
            am_cooling = true;
            targetTemperature.editable = false;
            SDK::SetTemperature( targetTemperature() );
            #ifdef NO_COOLER
            std::cerr << "Would cool if I were allowed to\n";
            #else
            CoolerON();
            #endif
        } else if ( !doCool() && am_cooling ) {
            if ( ! expect_doCool_change ) {
                DEBUG("Requesting state machine to be at most "
                        "connected");
                sm.ensure_at_most(States::Connected, StateMachine::User)
                    ->wait_for_fulfillment();
                DEBUG("Request fulfilled");
            }
            am_cooling = false;
            #ifdef NO_COOLER
            std::cerr << "Would stop cooling if I were allowed to\n";
            #else
            CoolerOFF();
            #endif
            targetTemperature.editable = true;
        }
    }
    else if ( &e.source == &targetTemperature.value ) {
        requiredTemperature.setMin( targetTemperature() + 1 );
    }
}

void Temperature::cool() 
{
    DEBUG("Preparing cooling transition, cooler is " << doCool());
    /* Make sure the cooling switch is pressed. If pressed already,
     * nothing happens. */
    Setter<bool> setter(expect_doCool_change, true);
    simparm::AttributeChange<bool> change( doCool.value, true );
    Setter<bool> resetter(expect_doCool_change, false);

    /* We need to check for the existence of the \c temperatureMonitor
    * here because finalize() stops that process, not stopCooling().
    * Thus, it might still be active if we went through the states
    * Inquired -> Initialized -> Cooled -> Initialized . */

    DEBUG("Began cooling transition");
    #ifndef NO_COOLER
    sm.status = "Waiting to reach required acquisition temperature";
    std::pair<bool, float> tstate;
    do {
        tstate = GetTemperatureF();
        sm.wait_or_abort_transition( StateMachine::Up, 100 );
    } while ( tstate.first == false &&
              tstate.second > requiredTemperature() &&
              !dStorm::ErrorHandler::global_termination_flag() );
    sm.status = "Reached required temperature"; 
    #endif
    DEBUG("Finished transition");
    change.release();
}

void Temperature::warm() 
{
    Setter<bool> setter(expect_doCool_change, true);
    simparm::AttributeChange<bool> change( doCool.value, false );
    Setter<bool> resetter(expect_doCool_change, false);

    #ifndef NO_COOLER
    std::pair<bool, float> tstate;
    do {
        tstate = GetTemperatureF();
        realTemperature = tstate.second;
        sm.wait_or_abort_transition( StateMachine::Down, 100 );
    } while ( tstate.second < -20 );
    #endif
    monitor.reset(NULL);
    change.release();
}

MK_EMPTY_RW(Temperature)

template <>
class Temperature::Token<Connected> 
: public States::Token
{
    Temperature& t;
    simparm::EditabilityChanger e;
    simparm::VisibilityChanger v;
  public:
    Token(Temperature& t) : t(t), e(t.doCool, t.is_active), 
                                  v(t.doCool, true) 
    {
        /* Read temperature range */
        std::pair<int,int> temp_range;
        temp_range = GetTemperatureRange();
        t.targetTemperature.setMin(temp_range.first);
        t.targetTemperature.setMax(temp_range.second);
        t.requiredTemperature.setMax(temp_range.second);

        t.monitor.reset( new TemperatureMonitor(t.realTemperature) );
    }
};

template <>
Temperature::Token<Connecting>::Token(Temperature& t) : parent(t) {
    if ( t.is_active )
        try {
            SDK::CoolerOFF();
        } catch ( const Error& ) {}
}
template <>
Temperature::Token<Connecting>::~Token() {
    if ( parent.is_active ) {
        parent.sm.status = "Warming up";
        parent.warm(); 
        parent.sm.status = "Warmed up";
    }
}

template <>
class Temperature::Token<Acquiring> 
: public States::Token
{
    Temperature& t;
    simparm::UsabilityChanger c;
    simparm::EditabilityChanger e;

  public:
    Token(Temperature& t) : t(t), c(t.doCool, false), 
                            e(t.requiredTemperature, false) 
    {
        DEBUG("Destructing temperature monitor");
        t.monitor.reset( NULL );
        DEBUG("Destructed temperature monitor");
    }

    ~Token() {
        DEBUG("Reconstructing temperature monitor");
        t.monitor.reset( new TemperatureMonitor(t.realTemperature) );
        DEBUG("Reconstructing temperature monitor");
    }
};

template <>
Temperature::Token<Readying>::Token(Temperature &t) 
: parent(t) 
{
    t.cool();
}

template class StateMachine::StandardListener<Temperature>;

}
