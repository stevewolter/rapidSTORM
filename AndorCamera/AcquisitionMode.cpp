#include "debug.h"
#include "AcquisitionMode.h"
#include "SDK.h"
#include <string.h>
#include "Config.h"
#include "System.h"

#include <simparm/ChoiceEntry_Impl.hh>
#include "StateMachine_impl.h"
#include <simparm/EntryManipulators.hh>

#include <simparm/OptionalEntry_impl.hh>

using namespace simparm;

namespace AndorCamera {

using boost::units::si::seconds;
using namespace States;

AcquisitionModeControl::AcquisitionModeControl(StateMachine& sm, Config& config)

: simparm::Object("AcquisitionMode", "Acquisition mode"),
  StateMachine::StandardListener<AcquisitionModeControl>(*this),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  sm(sm),
  real_exposure_time(config.realExposureTime),
  real_kinetic_cycle_time(config.cycleTime)
{
    registerNamedEntries();
}

AcquisitionModeControl::AcquisitionModeControl(const AcquisitionModeControl& c):
  simparm::Object(c),
  _AcquisitionMode(c),
  StateMachine::StandardListener<AcquisitionModeControl>(*this),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  sm(c.sm),
  real_exposure_time(c.real_exposure_time),
  real_kinetic_cycle_time(c.real_kinetic_cycle_time)
{
    registerNamedEntries();
}

_AcquisitionMode::_AcquisitionMode() :
  select_mode("SelectAcquisitionMode", "Select acquisition mode"),
  desired_exposure_time("DesiredExposureTime", 
    "Desired exposure time (s)", 0.1f * seconds),
  desired_accumulate_cycle_time(
    "DesiredAccumulateCycleTime",
        "Desired accumulate cycle time (s)", 0.1f * seconds),
  real_accumulate_cycle_time("RealAccumulateCycleTime",
    "Used accumulate cycle time (s)", 0.0f * seconds),
  number_of_accumulations("AccumulationNumber", 
    "Number of accumulations per kinetic cycle", 1),
  desired_kinetic_cycle_time("DesiredKineticCycleTime",
    "Desired kinetic cycle time (s)", 0.1f * seconds),
  kinetic_length("KineticLength", "Length of kinetic series")
{
    kinetic_length = 8000 * cs_units::camera::frame;
    kinetic_length().reset();

    select_mode.addChoice(Single_Scan, "SingleScan", "Single scan");
    select_mode.addChoice(Accumulate, "Accumulate", "Accumulate");
    select_mode.addChoice(Kinetics, "Kinetics", "Kinetics");
    select_mode.addChoice(Fast_Kinetics, "FastKinetics", "Fast kinetics");
    select_mode.addChoice(Run_till_abort, "RunTillAbort", "Run till abort");
    select_mode.addChoice(Time_Delayed_Integration, 
                          "TimeDelayedIntegration", "Time delayed integration");
    select_mode = Kinetics;

    real_accumulate_cycle_time.viewable = false;
    real_accumulate_cycle_time.editable = false;
    real_accumulate_cycle_time.setUserLevel(simparm::Object::Intermediate);
    desired_exposure_time.setUserLevel
        (simparm::Object::Intermediate);
    desired_accumulate_cycle_time.setUserLevel
        (simparm::Object::Intermediate);
    number_of_accumulations.setUserLevel(simparm::Object::Intermediate);

    desired_accumulate_cycle_time.setMax( 
        desired_kinetic_cycle_time() / (1.0f * number_of_accumulations()) );
    desired_exposure_time.setMax( desired_accumulate_cycle_time() );

}

void AcquisitionModeControl::operator()
    (const simparm::Event& e) 
{
    if ( &e.source == &desired_exposure_time.value ) {
        exp_time_is_max = ( desired_exposure_time() * 1.05f >= 
             desired_accumulate_cycle_time() );
    } else if ( &e.source == &desired_accumulate_cycle_time.value ) {
        acc_time_is_max = ( desired_accumulate_cycle_time() * 1.05f >= 
             desired_kinetic_cycle_time() / (1.0f * number_of_accumulations()) );

        desired_exposure_time.setMax( 
            desired_accumulate_cycle_time() );
    } else if ( &e.source == &desired_kinetic_cycle_time.value ) {
        desired_accumulate_cycle_time.setMax( 
            desired_kinetic_cycle_time() / (1.0f * number_of_accumulations()) );
    } else if ( &e.source == &desired_accumulate_cycle_time.max ) {
        if ( acc_time_is_max )
            desired_accumulate_cycle_time.value
                = desired_accumulate_cycle_time.max();
    } else if ( &e.source == &desired_exposure_time.max ) {
        if ( exp_time_is_max )
            desired_exposure_time.value
                = desired_exposure_time.max();
    }
}

AcquisitionModeControl::~AcquisitionModeControl()
{
    DEBUG("Deleting acquisition mode control " << this);
}

void AcquisitionModeControl::registerNamedEntries() 
{
    exp_time_is_max = ( desired_exposure_time() * 1.05f >= 
            desired_accumulate_cycle_time() );
    acc_time_is_max = ( desired_accumulate_cycle_time() * 1.05f >= 
            desired_kinetic_cycle_time() / (1.0f * number_of_accumulations()) );

    receive_changes_from( desired_exposure_time.value );
    receive_changes_from( desired_accumulate_cycle_time.value );
    receive_changes_from( desired_kinetic_cycle_time.value );
    receive_changes_from( desired_exposure_time.max );
    receive_changes_from( desired_accumulate_cycle_time.max );
    receive_changes_from( desired_kinetic_cycle_time.max );

    push_back( desired_exposure_time );
    push_back( real_exposure_time );
    push_back( number_of_accumulations );
    push_back( desired_accumulate_cycle_time );
    push_back( real_accumulate_cycle_time );
    push_back( desired_kinetic_cycle_time );
    push_back( real_kinetic_cycle_time );
    push_back( kinetic_length );
}

MK_EMPTY_RW(AcquisitionModeControl)

class AcquisitionModeControl::ManagedAcquisition {
    EditabilityChanger sm, det, dkt, kl, na, dact;
  public:
    ManagedAcquisition(AcquisitionModeControl& a)
        : sm(a.select_mode, false), det(a.desired_exposure_time, false),
          dkt(a.desired_kinetic_cycle_time, false), 
          kl(a.kinetic_length, false), 
          na(a.number_of_accumulations, false),
          dact(a.desired_accumulate_cycle_time, false)
    {
        SDK::SetAcquisitionMode( a.select_mode() );
        SDK::SetExposureTime( a.desired_exposure_time() / seconds );

        /* TODO: Accumulate cycle time. */
        if ( a.select_mode() == Kinetics 
             || a.select_mode() == Fast_Kinetics
             || a.select_mode() == Run_till_abort )
            SDK::SetKineticCycleTime( a.desired_kinetic_cycle_time() / seconds );
        if ( a.kinetic_length().is_set() && 
             (a.select_mode() == Kinetics || a.select_mode() == Fast_Kinetics ) )
            SDK::SetNumberKinetics( *a.kinetic_length() / cs_units::camera::frame );
    }
};

template <>
class AcquisitionModeControl::Token<Readying>
: public States::Token
{
    std::auto_ptr<ManagedAcquisition> man;
  public:
    Token( AcquisitionModeControl& a ) {
        if ( a.is_active ) {
            man.reset( new ManagedAcquisition(a) );
        }
    }
};

template <>
class AcquisitionModeControl::Token<Ready>
: public States::Token
{
    simparm::VisibilityChanger v1, v2, v3;

  public:
    Token( AcquisitionModeControl& a ) 
        : v1(a.real_exposure_time, true),
          v2(a.real_accumulate_cycle_time, true),
          v3(a.real_kinetic_cycle_time, true) 
    {
        a.real_exposure_time = SDK::GetExposureTime() * seconds;
        a.real_accumulate_cycle_time = SDK::GetAccumulationCycleTime() * seconds;
        a.real_kinetic_cycle_time = SDK::GetKineticCycleTime() * seconds;
    }
};

template class StateMachine::StandardListener<AcquisitionModeControl>;
}
