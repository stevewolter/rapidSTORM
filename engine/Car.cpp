#define DSTORM_CAR_CPP
#include "debug.h"

#include "Car.h"
#include <dStorm/outputs/Crankshaft.h>
#include <dStorm/ImageTraits.h>
#include <dStorm/input/Source.h>
#include <dStorm/output/Localizations.h>
#include <dStorm/engine/Image.h>
#include <dStorm/input/Config.h>
#include <fstream>
#include <queue>
#include <dStorm/output/Output.h>
#include "doc/help/context.h"
#include <dStorm/localization_file/reader.h>
#include <dStorm/engine/Image.h>
#include <dStorm/helpers/OutOfMemory.h>
#include <dStorm/helpers/exception.h>
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/error_handler.h>

extern void test_exception_site();

using namespace std;

using dStorm::output::Output;

namespace dStorm {
namespace engine {

class Car::ComputationThread : public ost::Thread {
  private:
    Car &car;
    auto_ptr<string> nm;

  public:
    ComputationThread(Car &car, 
                 auto_ptr<string> name) 
        : ost::Thread(name->c_str()), 
          car(car), nm(name) {}
    ~ComputationThread() {
        DEBUG("Collecting piston");
        join(); 
    }
    void run() throw() {
        car.run_computation();
    }
    void abnormal_termination(std::string r) {
        std::cerr << "Computation thread " << *nm 
                  << " in job " << car.ident
                  << " had a critical error: " << r
                  << " Terminating job computation."
                  << std::endl;
        car.stop();
    }
};

/* ==== This code gives a new job ID on each call. === */
static ost::Mutex *runNumberMutex = NULL;
static char number[6];
static std::string getRunNumber() {
    DEBUG("Making run number");
    if ( ! runNumberMutex) {
        runNumberMutex = new ost::Mutex();
        strcpy(number, "   00");
    }
    ost::MutexLock lock(*runNumberMutex);

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

Car::Car (JobMaster* input_stream, const dStorm::Config &new_config) 
: ost::Thread("Car"),
  simparm::Listener( simparm::Event::ValueChanged ),
  input_stream( input_stream ),
  config(new_config),
  ident( getRunNumber() ),
  runtime_config("dStormJob" + ident, "dStorm Job " + ident),
  abortJob("StopComputation", "Stop computation"),
  closeJob("CloseJob", "Close job"),
  input(NULL),
  output(NULL),
  terminate( new_config.auto_terminate() ),
  emergencyStop(false), error(false), finished(false),
  terminationChanged( terminationMutex )
{
    //DEBUG("Building car from config " << &config << " and meta info " << &(config.get_meta_info()) );
    used_output_filenames = config.get_meta_info().forbidden_filenames;
    closeJob.helpID = HELP_CloseJob;
    abortJob.helpID = HELP_StopEngine;

    receive_changes_from( abortJob.value );
    receive_changes_from( closeJob.value );
    receive_changes_from( runtime_config );

    DEBUG("Determining input file name from basename " << config.get_meta_info().suggested_output_basename.new_basename());
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
        this->input_stream->register_node( *this );
}

Car::~Car() 
{
    DEBUG("Destructing Car");
    DEBUG("Joining car subthread");
    join();

    DEBUG("Sending destruction signal to outputs");
    output->propagate_signal( Output::Prepare_destruction );

    DEBUG("Removing from input_stream config");
    /* Remove from simparm parents to hide destruction process
     * from interface. */
    if ( input_stream )
        input_stream->erase_node( *this );

    DEBUG("Deleting outputs");
    output.reset(NULL);
    DEBUG("Deleting input");
    input.reset(NULL);
    DEBUG("Commencing destruction");
}

void Car::operator()(const simparm::Event& e) {
    if ( &e.source == &closeJob.value && e.cause == simparm::Event::ValueChanged && closeJob.triggered() )
    {
        closeJob.untrigger();
        closeJob.editable = false;
        DEBUG("Close job pressed, locking for job termination");
        ost::MutexLock lock( terminationMutex );
        DEBUG("Job close button allows termination" );
        terminate = true;
        emergencyStop = error = true;
        terminationChanged.signal();
    } else if ( &e.source == &abortJob.value && e.cause == simparm::Event::ValueChanged && abortJob.triggered() )
    {
        DEBUG("Abort job button pressed");
        abortJob.untrigger();
        abortJob.editable = false;
        error = false;
        emergencyStop = finished = true;
    }
}

void Car::run() throw() {
    try {
        drive();
    } catch ( const std::bad_alloc& e ) {
        OutOfMemoryMessage m("Job " + ident);
        runtime_config.send(m);
    } catch ( const dStorm::runtime_error& e ) {
        simparm::Message m( e.get_message("Error in Job " + ident) );
        runtime_config.send(m);
    } catch ( const std::exception& e ) {
        simparm::Message m("Error in Job " + ident, 
                               "Job " + ident + " failed: " + e.what() );
        runtime_config.send(m);
    }
}

void Car::add_thread()
{
    int pistonCount = threads.size();
    std::auto_ptr<string> pistonName( new string("Piston 00") );
    (*pistonName)[7] += pistonCount / 10;
    (*pistonName)[8] += pistonCount % 10;
    std::auto_ptr<ComputationThread> new_piston
        ( new ComputationThread(*this, pistonName) );
    new_piston->start();
    threads.push_back( new_piston );
}

void Car::compute_until_terminated() {
    int number_of_threads = 1;
    if ( input->flags.test( input::BaseSource::MultipleConcurrentIterators ) )
        number_of_threads = config.pistonCount();

    while (true) {
        DEBUG("Announcing run");
        Output::RunRequirements r = 
            output->announce_run(Output::RunAnnouncement());
        if ( ! r.test(Output::MayNeedRestart) )
            input->dispatch( Input::WillNeverRepeatAgain );

        DEBUG("Adding threads");
        for (int i = /* --> */ 1 /* <-- */; i < number_of_threads; ++i)
            add_thread();
        DEBUG("Running computation");
        run_computation();
        DEBUG("Collecting threads");
        threads.clear();
        DEBUG("Collected threads");

        if (emergencyStop) {
            if (error || dStorm::ErrorHandler::global_termination_flag() ) 
            {
                DEBUG("Emergency stop due to error or global termination");
                output->propagate_signal( Output::Engine_run_failed );
                break;
            } else if ( finished ) {
                DEBUG("Emergency stop due to user interaction");
                output->propagate_signal( Output::Engine_run_succeeded );
                break;
            } else {
                DEBUG("Emergency stop for restart");
                emergencyStop = false;
                input->dispatch( input::BaseSource::RepeatInput );
                output->propagate_signal( Output::Engine_is_restarted );
            }
        } else {
            DEBUG("Job ended, no emergency stop");
            output->propagate_signal( Output::Engine_run_succeeded );
            break;
        }
    }
}

void Car::run_computation() 
{
    try {
        DEBUG("Propagating start signal");
        output->propagate_signal(Output::Engine_run_is_starting);

        DEBUG("Running computation loop");
        for (Input::iterator i = input->begin(), e = input->end(); i != e; ++i) 
        {
            DEBUG("Computation loop iteration");
            Output::Result r = 
                output->receiveLocalizations( *i );
            
            if (r == Output::RestartEngine || r == Output::StopEngine ) 
            {
                DEBUG("Emergency stop: Engine restart requested");
                output->propagate_signal( Output::Engine_run_is_aborted );
                emergencyStop = true;
                if ( r == Output::StopEngine )
                    error = true;
            }

            if (emergencyStop || ErrorHandler::global_termination_flag()) 
            {
                DEBUG("Emergency stop: " << emergencyStop << " " << ErrorHandler::global_termination_flag());
                break;
            } else {
                DEBUG("Continuing with computation");
            }
        }
        DEBUG("Reached end of computation");
        return;
    } catch (const dStorm::abort&) {
    } catch (const std::bad_alloc& e) {
        OutOfMemoryMessage m("Job " + ident);
        runtime_config.send(m);
    } catch ( const dStorm::runtime_error& e ) {
        simparm::Message m( e.get_message("Error in Job " + ident) );
        runtime_config.send(m);
    } catch (const std::exception& e) {
        simparm::Message m("Error in Job " + ident, e.what() );
        runtime_config.send(m);
    } catch (...) {
        simparm::Message m("Unspecified error", "Unknown type of failure. Sorry." );
        runtime_config.send( m );
    }
    emergencyStop = error = true;
}

void Car::add_additional_outputs() {
    boost::ptr_vector<output::Output> o = input->additional_outputs();

    if ( ! o.empty() ) 
    {
        outputs::Crankshaft *crankshaft = 
            dynamic_cast<outputs::Crankshaft*>(output.get());
        if ( crankshaft == NULL )  {
            std::auto_ptr<outputs::Crankshaft> 
                temporaryCrankshaft( new outputs::Crankshaft() );
            crankshaft = temporaryCrankshaft.get();

            temporaryCrankshaft->add( output );
            this->output.reset( temporaryCrankshaft.release() );
        }

        while ( ! o.empty() )
            crankshaft->add( o.pop_back().release() );
    }
}

void Car::drive() {
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

    add_additional_outputs();

    DEBUG("Input has config node " << static_cast<simparm::Node&>(*input).getName());
    DEBUG("Pushing back input config");
    runtime_config.push_back( *input );
    DEBUG("Pushing back output config");
    runtime_config.push_back( *output );
    DEBUG("Pushing back abort job button");
    runtime_config.push_back( abortJob );
    DEBUG("Pushing back close job button");
    runtime_config.push_back( closeJob );

    DEBUG("Creating announcement");
    Output::Announcement announcement( *input->get_traits() );
    DEBUG("Sending announcement");
    Output::AdditionalData data 
        = output->announceStormSize(announcement);
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
        compute_until_terminated();
    }

    DEBUG("Erasing input");
    input.reset(NULL);
    DEBUG("Erased input");
  } catch (const dStorm::abort&) {
    DEBUG("Caught abortion signal");
  } catch (const std::bad_alloc& e) {
    OutOfMemoryMessage m("Job " + ident);
    runtime_config.send(m);
  } catch (const dStorm::runtime_error& e) {
    simparm::Message m( e.get_message("Error in Job " + ident) );
    runtime_config.send(m);
  } catch (const std::exception& e) {
    simparm::Message m("Error in Job " + ident, e.what() );
    runtime_config.send(m);
  } catch (...) {
    simparm::Message m("Unspecified error", "Unknown type of failure. Sorry." );
    runtime_config.send( m );
  }

    ost::MutexLock lock( terminationMutex );
    DEBUG("Waiting for termination allowance");
    while ( ! terminate )
        terminationChanged.wait();
    DEBUG("Allowed to terminate");

    /* TODO: We have to check here if the job was _really_ finished
    * successfully. */
    output->propagate_signal( 
        Output::Job_finished_successfully );

    if ( config.configTarget ) {
        std::ostream& stream = config.configTarget.get_output_stream();
        std::list<string> lns = config.printValues();
        for (std::list<string>::const_iterator i = lns.begin(); i != lns.end(); i++)
            stream << *i << "\n";
        config.configTarget.close_output_stream();
    }
}

void Car::stop() {
    abortJob.trigger();
    closeJob.trigger();
}

void Car::abnormal_termination(std::string r) throw() {
    std::cerr << "Job " << ident << " had a critical "
              << "error: " << r << " Terminating job."
              << std::endl;
}

}
}

