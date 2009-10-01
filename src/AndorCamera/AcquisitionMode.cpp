#include "AcquisitionMode.h"
#include "SDK.h"
#include <string.h>
#include "Config.h"
#include "System.h"

#include <simparm/ChoiceEntry_Impl.hh>

using namespace simparm;

namespace AndorCamera {

using namespace States;
using namespace Phases;

AcquisitionModeControl::AcquisitionModeControl(StateMachine& sm, Config& config)

: simparm::Object("AcquisitionMode", "Acquisition mode"),
  sm(sm),
  real_exposure_time(config.realExposureTime),
  real_kinetic_cycle_time(config.cycleTime)
{
    registerNamedEntries();
}

AcquisitionModeControl::AcquisitionModeControl(const AcquisitionModeControl& c):
  simparm::Node(c),
  simparm::Object(c),
  _AcquisitionMode(c),
  Listener(),
  simparm::Node::Callback(c),
  sm(c.sm),
  real_exposure_time(c.real_exposure_time),
  real_kinetic_cycle_time(c.real_kinetic_cycle_time)
{
    registerNamedEntries();
}

_AcquisitionMode::_AcquisitionMode() :
  select_mode("SelectAcquisitionMode", "Select acquisition mode"),
  desired_exposure_time("DesiredExposureTime", 
    "Desired exposure time (s)", 0.1),
  desired_accumulate_cycle_time(
    "DesiredAccumulateCycleTime",
        "Desired accumulate cycle time (s)", 0.1),
  real_accumulate_cycle_time("RealAccumulateCycleTime",
    "Used accumulate cycle time (s)", 0),
  number_of_accumulations("AccumulationNumber", 
    "Number of accumulations per kinetic cycle", 1),
  desired_kinetic_cycle_time("DesiredKineticCycleTime",
    "Desired kinetic cycle time (s)", 0.1),
  kinetic_length("KineticLength",
    "Length of kinetic series", 8000)
{
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
    real_accumulate_cycle_time.setUserLevel(simparm::Entry::Intermediate);
    desired_exposure_time.setUserLevel
        (simparm::Entry::Intermediate);
    desired_accumulate_cycle_time.setUserLevel
        (simparm::Entry::Intermediate);
    number_of_accumulations.setUserLevel(simparm::Entry::Intermediate);

    desired_accumulate_cycle_time.setMax( 
        desired_kinetic_cycle_time() / number_of_accumulations() );
    desired_exposure_time.setMax( desired_accumulate_cycle_time() );

}

void AcquisitionModeControl::operator()
    (Node &src, Node::Callback::Cause c, Node *) 
{
    if ( c != ValueChanged ) return;

    if ( &src == &desired_exposure_time.value ) {
        exp_time_is_max = ( desired_exposure_time() * 1.05 >= 
             desired_accumulate_cycle_time() );
    } else if ( &src == &desired_accumulate_cycle_time.value ) {
        acc_time_is_max = ( desired_accumulate_cycle_time() * 1.05 >= 
             desired_kinetic_cycle_time() / number_of_accumulations() );

        desired_exposure_time.setMax( 
            desired_accumulate_cycle_time() );
    } else if ( &src == &desired_kinetic_cycle_time.value ) {
        desired_accumulate_cycle_time.setMax( 
            desired_kinetic_cycle_time() / number_of_accumulations() );
    } else if ( &src == &desired_accumulate_cycle_time.max ) {
        if ( acc_time_is_max )
            desired_accumulate_cycle_time.value
                = desired_accumulate_cycle_time.max();
    } else if ( &src == &desired_exposure_time.max ) {
        if ( exp_time_is_max )
            desired_exposure_time.value
                = desired_exposure_time.max();
    }
}

AcquisitionModeControl::~AcquisitionModeControl()
{
    PROGRESS("Deleting acquisition mode control " << this);
    sm.remove_managed_attribute( real_exposure_time.viewable );
    sm.remove_managed_attribute( real_accumulate_cycle_time.viewable );
    sm.remove_managed_attribute( real_kinetic_cycle_time.viewable );
}

static void show_info(StateMachine &sm, simparm::Entry &e) {
    sm.add_managed_attribute( e.viewable, Acquiring );
}

void AcquisitionModeControl::registerNamedEntries() 
{
    exp_time_is_max = ( desired_exposure_time() * 1.05 >= 
            desired_accumulate_cycle_time() );
    acc_time_is_max = ( desired_accumulate_cycle_time() * 1.05 >= 
            desired_kinetic_cycle_time() / number_of_accumulations() );

    receive_changes_from( desired_exposure_time.value );
    receive_changes_from( desired_accumulate_cycle_time.value );
    receive_changes_from( desired_kinetic_cycle_time.value );
    receive_changes_from( desired_exposure_time.max );
    receive_changes_from( desired_accumulate_cycle_time.max );
    receive_changes_from( desired_kinetic_cycle_time.max );

    show_info( sm, real_exposure_time );
    show_info( sm, real_accumulate_cycle_time );
    show_info( sm, real_kinetic_cycle_time );

    push_back( desired_exposure_time );
    push_back( real_exposure_time );
    push_back( number_of_accumulations );
    push_back( desired_accumulate_cycle_time );
    push_back( real_accumulate_cycle_time );
    push_back( desired_kinetic_cycle_time );
    push_back( real_kinetic_cycle_time );
    push_back( kinetic_length );
}

void AcquisitionModeControl::controlStateChanged( Phase phase, State from, State to)

{
    if ( phase == Transition && to == Acquiring ) 
    {
        select_mode.editable = false;
        SDK::SetAcquisitionMode( select_mode() );

        desired_exposure_time.editable = false;
        SDK::SetExposureTime( desired_exposure_time() );

        /* TODO: Accumulate cycle time. */
        desired_kinetic_cycle_time.editable = false;
        if ( select_mode() == Kinetics || select_mode() == Fast_Kinetics
             || select_mode() == Run_till_abort )
            SDK::SetKineticCycleTime( desired_kinetic_cycle_time() );
        kinetic_length.editable = false;
        if ( select_mode() == Kinetics || select_mode() == Fast_Kinetics )
            SDK::SetNumberKinetics( kinetic_length() );

        number_of_accumulations.editable = false;
        desired_accumulate_cycle_time.editable = false;
    } else if ( phase == Transition && from == Acquiring ) {
        select_mode.editable = true;
        desired_exposure_time.editable = true;
        desired_kinetic_cycle_time.editable = true;
        kinetic_length.editable = true;
        number_of_accumulations.editable = true;
        desired_accumulate_cycle_time.editable = true;
    } else if ( phase == Review && to == Acquiring ) {
        real_exposure_time = SDK::GetExposureTime();
        real_accumulate_cycle_time = SDK::GetAccumulationCycleTime();
        real_kinetic_cycle_time = SDK::GetKineticCycleTime();
    }
}

}
