#include "debug.h"

#include "Car.h"
#include "Run.h"
#include <dStorm/ImageTraits.h>
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

Car::Car (JobMaster* input_stream, const Config &new_config) 
: simparm::Listener( simparm::Event::ValueChanged ),
  config(new_config),
  ident( getRunNumber() ),
  runtime_config("dStormJob" + ident, "dStorm Job " + ident),
  abortJob("StopComputation", "Stop computation"),
  closeJob("CloseJob", "Close job"),
  input(NULL),
  output(NULL),
  close_job( new_config.auto_terminate() ),
  abort_job( false )
{
    //DEBUG("Building car from config " << &config << " and meta info " << &(config.get_meta_info()) );
    used_output_filenames = config.get_meta_info().forbidden_filenames;
    closeJob.helpID = "#CloseJob";
    abortJob.helpID = "#StopEngine";

    receive_changes_from( abortJob.value );
    receive_changes_from( closeJob.value );
    receive_changes_from( runtime_config );

    output::Basename bn( config.get_meta_info().suggested_output_basename );
    bn.set_variable("run", ident);
    DEBUG("Setting output basename to " << bn.unformatted()() << " (expanded " << bn.new_basename() << ")");
    config.outputSource.set_output_file_basename( bn );
    DEBUG("Building output");
    output = config.outputSource.make_output();
    if ( output.get() == NULL )
        throw std::invalid_argument("No valid output supplied.");
    DEBUG("Checking for duplicate filenames");
    output->check_for_duplicate_filenames( used_output_filenames );
    DEBUG("Checked for duplicate filenames");

    DEBUG("Registering at input_stream config " << input_stream);
    if ( input_stream )
        job_handle = input_stream->register_node( *this );

    master_thread = boost::thread( &Car::run, this );
}

Car::~Car() 
{
    DEBUG("Destructing Car");
    DEBUG("Joining car subthread");
    master_thread.join();

    output->prepare_destruction();

    if ( job_handle.get() != NULL )
        job_handle->unregister_node();

    DEBUG("Deleting outputs");
    output.reset();
    DEBUG("Deleting input");
    input.reset(NULL);
    DEBUG("Commencing destruction");
}

void Car::operator()(const simparm::Event& e) {
    if ( &e.source == &closeJob.value && e.cause == simparm::Event::ValueChanged && closeJob.triggered() )
    {
        closeJob.untrigger();
        closeJob.editable = false;
        DEBUG("Job close button allows termination" );
        boost::lock_guard<boost::recursive_mutex> lock( mutex );
        close_job = true;
        abort_job = true;
        computation_thread.interrupt();
        allow_termination.notify_all();
    } else if ( &e.source == &abortJob.value && e.cause == simparm::Event::ValueChanged && abortJob.triggered() )
    {
        DEBUG("Abort job button pressed");
        abortJob.untrigger();
        abortJob.editable = false;
        boost::lock_guard<boost::recursive_mutex> lock( mutex );
        abort_job = true;
        computation_thread.interrupt();
    }
}

void Car::run() {
    try {
        drive();
    } catch ( const std::bad_alloc& e ) {
        OutOfMemoryMessage m("Job " + ident);
        runtime_config.send(m);
    } catch ( const std::runtime_error& e ) {
        DEBUG("Sending message in run loop " << e.what());
        simparm::Message m("Error in Job " + ident, 
                               "Job " + ident + " failed: " + e.what() );
        runtime_config.send(m);
        DEBUG("Sent message in run loop " << e.what());
    }

    master_thread.detach();
    delete this;
}

void Car::compute() {
  try {
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

    runtime_config.push_back( *input );
    runtime_config.push_back( output->getNode() );
    runtime_config.push_back( abortJob );
    runtime_config.push_back( closeJob );

    input::BaseSource::Wishes requirements;
    if ( config.pistonCount() > 1 )
        requirements.set( input::BaseSource::ConcurrentIterators );

    DEBUG("Getting input traits from " << input.get());
    Input::TraitsPtr traits = input->get_traits(requirements);
    first_output = *traits->image_number().range().first;
    DEBUG("Job length declared as " << traits->image_number().range().second.get_value_or( -1 * camera::frame ) );
    DEBUG("Creating announcement from traits " << traits.get());
    Output::Announcement announcement( *traits, display::Manager::getSingleton() );
    upstream_engine = announcement.engine;
    announcement.engine = this;
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
                "The selected input module cannot provide localization traces."
                "Please select an appropriate output.");
        runtime_config.send(m);
        return;
    } else if ( data.test( output::Capabilities::SourceImage ) &&
                ! announcement.source_image_is_set )
    {
        simparm::Message m("Unable to provide data",
                   "One of your output modules needs access to the raw " +
                   std::string("images of the acquisition. These are not present in ") +
                   "the input. Either remove the output or " +
                   "choose a different input file or method.");
        runtime_config.send(m);
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
        runtime_config.send(m);
    } else {
        int number_of_threads = 1;
        if ( input->capabilities().test( input::BaseSource::ConcurrentIterators ) )
            number_of_threads = config.pistonCount();
        while ( ! abort_job ) {
            current_run.reset( new Run( mutex, first_output, *input, *output,
                                        number_of_threads ) );
            Run::Result result = current_run->run();
            if ( result == Run::Succeeded ) {
                boost::lock_guard< boost::recursive_mutex > lock(mutex);
                output->store_results();
                break;
            } else if ( result == Run::Failed ) {
                break;
            } else {
                current_run.reset();
                input->dispatch( input::BaseSource::RepeatInput );
            }
        }
    }

    if ( config.configTarget ) {
        std::ostream& stream = config.configTarget.get_output_stream();
        std::list<std::string> lns = config.printValues();
        for (std::list<std::string>::const_iterator i = lns.begin(); i != lns.end(); i++)
            stream << *i << "\n";
        config.configTarget.close_output_stream();
    }

  } catch ( boost::thread_interrupted ) {
  } catch (const std::bad_alloc& e) {
    OutOfMemoryMessage m("Job " + ident);
    runtime_config.send(m);
  } catch (const std::runtime_error& e) {
    DEBUG("Sending message in drive mode");
    simparm::Message m( "Error in Job " + ident, e.what() );
    runtime_config.send(m);
    DEBUG("Sent message in drive mode");
  }
}

void Car::drive() {
    if ( ! abort_job ) {
        computation_thread = boost::thread( &Car::compute, this );
        computation_thread.join();
    }
    current_run.reset();
    input.reset();
    boost::unique_lock<boost::recursive_mutex> lock( mutex );
    DEBUG("Waiting for termination allowance");
    while ( ! close_job )
        allow_termination.wait(lock);
    DEBUG("Allowed to terminate");

}

void Car::stop() {
    abortJob.trigger();
    closeJob.trigger();
}

void Car::restart() {
    current_run->restart();
}

void Car::repeat_results() {
    throw std::logic_error("Cannot repeat results from dSTORM car");
}
bool Car::can_repeat_results() { return false; }

void Car::change_input_traits( std::auto_ptr< input::BaseTraits > new_traits )
{
    if ( upstream_engine )
        upstream_engine->change_input_traits( new_traits );
}

std::auto_ptr<EngineBlock> Car::block() {
    return current_run->block();
}

}
}

