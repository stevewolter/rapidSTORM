#include "debug.h"

#include "Car.h"
#include "Run.h"
#include <dStorm/image/MetaInfo.h>
#include <dStorm/input/Source.h>
#include <dStorm/output/Localizations.h>
#include <dStorm/engine/Image.h>
#include <fstream>
#include <queue>
#include <dStorm/output/Output.h>
#include <dStorm/engine/Image.h>
#include <dStorm/helpers/OutOfMemory.h>
#include <dStorm/input/MetaInfo.h>
#include <dStorm/display/Manager.h>
#include <boost/smart_ptr/make_shared.hpp>
#include <ui/serialization/serialize.h>

using dStorm::output::Output;

namespace dStorm {
namespace job {

/* ==== This code gives a new job ID on each call. === */
static boost::mutex *runNumberMutex = NULL;
static char number[6];
static std::string getRunNumber() {
    DEBUG("Making run number");
    if ( ! runNumberMutex) {
        runNumberMutex = new boost::mutex();
        strcpy(number, "   00");
    }
    boost::lock_guard<boost::mutex> lock(*runNumberMutex);

    int index = strlen(number)-1;
    number[index]++;
    while (index >= 0)
        if (number[index] == '9'+1) {
            number[index] = '0';
            if ( index > 0 )
                number[--index]++;
        } else
            break;
    index = strlen(number)-1;
    while (index > 0 && isdigit(number[index-1])) index--;
    DEBUG("Made run number");
    return std::string(number+index);
}

Car::Car (const Config &config) 
: ident( getRunNumber() ),
  runtime_config("dStormJob" + ident, "dStorm Job " + ident),
  input(NULL),
  output(NULL),
  piston_count( config.pistonCount() ),
  control( config.auto_terminate() )
{
    used_output_filenames = config.get_meta_info().forbidden_filenames;

    DEBUG("Trying to make source");
    std::auto_ptr<input::BaseSource> rawinput( config.makeSource() );
    if ( ! rawinput.get() )
        throw std::logic_error("No input source was created");
    DEBUG("Made source");
    input.reset( dynamic_cast< Input* >(rawinput.get()) );
    if ( input.get() )
        rawinput.release();
    else 
        throw std::runtime_error("Engine output does not seem to be localizations");

    output::Basename bn( config.get_meta_info().suggested_output_basename );
    bn.set_variable("run", ident);
    DEBUG("Setting output basename");
    std::auto_ptr< output::OutputSource > 
        basename_adjusted_output( config.output_tree().clone() );
    basename_adjusted_output->set_output_file_basename( bn );
    DEBUG("Building output");
    output = basename_adjusted_output->make_output();
    if ( output.get() == NULL )
        throw std::invalid_argument("No valid output supplied.");
    DEBUG("Checking for duplicate filenames");
    output->check_for_duplicate_filenames( used_output_filenames );
    DEBUG("Checked for duplicate filenames");

    if ( config.configTarget ) {
        simparm::serialization_ui::serialize( config, config.configTarget() );
    }
}

Car::~Car() 
{
    output->prepare_destruction();

    DEBUG("Deleting outputs");
    output.reset();
    DEBUG("Deleting input");
    input.reset(NULL);
    DEBUG("Commencing destruction");
}

void Car::run() {
    try {
        drive();
    } catch ( const std::bad_alloc& e ) {
        OutOfMemoryMessage m("Job " + ident);
        m.send( current_ui );
    } catch ( const std::runtime_error& e ) {
        DEBUG("Sending message in run loop " << e.what());
        simparm::Message m("Error in Job " + ident, 
                               "Job " + ident + " failed: " + e.what() );
        m.send( current_ui );
        DEBUG("Sent message in run loop " << e.what());
    }
}

void Car::drive() {
  bool run_successful = false;
  try {
    input::BaseSource::Wishes requirements;
    if ( piston_count > 1 )
        requirements.set( input::BaseSource::ConcurrentIterators );

    DEBUG("Getting input traits from " << input.get());
    Input::TraitsPtr traits = input->get_traits(requirements);
    first_output = *traits->image_number().range().first;
    DEBUG("Job length declared as " << traits->image_number().range().second.get_value_or( -1 * camera::frame ) );
    DEBUG("Creating announcement from traits " << traits.get());
    Output::Announcement announcement( *traits );
    upstream_engine = announcement.engine;
    announcement.engine = &control;
    announcement.name = "Job" + ident;
    announcement.description = "Result image for Job " + ident;
    DEBUG("Sending announcement");
    Output::AdditionalData data;
    {
        boost::lock_guard<boost::recursive_mutex> lock(mutex);
        data = output->announceStormSize(announcement);
    }
    DEBUG("Sent announcement");

    if ( data.test( output::Capabilities::ClustersWithSources ) ) {
        simparm::Message m("Unable to provide data",
                "The selected input module cannot provide localization traces. "
                "Please select an appropriate output.");
        m.send( current_ui );
        return;
    } else if ( data.test( output::Capabilities::SourceImage ) &&
                ! announcement.source_image_is_set )
    {
        simparm::Message m("Unable to provide data",
                   "One of your output modules needs access to the raw " +
                   std::string("images of the acquisition. These are not present in ") +
                   "the input. Either remove the output or " +
                   "choose a different input file or method.");
        m.send( current_ui );
        return;
    } else if (
        ( data.test( output::Capabilities::SmoothedImage ) && 
          ! announcement.smoothed_image_is_set ) ||
        ( data.test( output::Capabilities::CandidateTree ) && 
          ! announcement.candidate_tree_is_set ) ||
        ( data.test( output::Capabilities::InputBuffer ) && 
          ! announcement.carburettor ) )
    {
        std::stringstream ss;
        ss << 
            "A selected data processing function "
            "requires data about the input data ("
            << data << ") that are not "
            "present in the current input.";
        simparm::Message m("Unable to provide data", ss.str());
        m.send( current_ui );
        return;
    }

    int number_of_threads = 1;
    if ( input->capabilities().test( input::BaseSource::ConcurrentIterators ) )
        number_of_threads = piston_count;
    bool run_succeeded = false;
    while ( ! run_succeeded && control.continue_computing() ) {
        current_run = boost::make_shared<Run>
            ( boost::ref(mutex), first_output, boost::ref(*input), boost::ref(*output), number_of_threads );
        control.set_current_run( current_run );
        Run::Result result = current_run->run();
        current_run.reset();
        control.set_current_run( current_run );

        if ( result == Run::Restart ) {
            if ( control.traits_changed() )
                upstream_engine->change_input_traits( control.new_traits() );
            input->dispatch( input::BaseSource::RepeatInput );
        } else if ( result == Run::Succeeded ) {
            run_succeeded = true;
        } else if ( result == Run::Failed ) {
            /* This should not happen, since any failure condition should
             * throw an exception. Abort, it seems the safest option. */
            throw boost::thread_interrupted();
        } else {
            assert( false );
        }
    }

    run_successful = ! control.aborted_by_user();

  } catch ( boost::thread_interrupted ) {
  } catch (const std::bad_alloc& e) {
    OutOfMemoryMessage m("Job " + ident);
    m.send( current_ui );
  } catch (const std::runtime_error& e) {
    simparm::Message m( "Error in Job " + ident, e.what() );
    m.send( current_ui );
  }

    try {
        output->store_results( run_successful );
    } catch (const std::bad_alloc& e) {
        OutOfMemoryMessage m("Job " + ident);
        m.send( current_ui );
    } catch (const std::runtime_error& e) {
        simparm::Message m( "Error while saving results of Job " + ident, e.what() );
        m.send( current_ui );
    }

    current_run.reset();
    control.set_current_run( current_run );
    input.reset();
    control.wait_until_termination_is_allowed();
}

void Car::stop() {
    control.stop();
}

simparm::NodeHandle Car::attach_ui( simparm::NodeHandle at ) {
    current_ui = runtime_config.attach_ui( at );
    input->attach_ui( current_ui );
    output->attach_ui( current_ui );
    control.registerNamedEntries( current_ui );
    return current_ui;
}


}
}

