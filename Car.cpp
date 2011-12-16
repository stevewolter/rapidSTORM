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
#include <dStorm/engine/Image.h>
#include <dStorm/helpers/OutOfMemory.h>
#include <dStorm/input/chain/MetaInfo.h>
#include <boost/range/algorithm/fill.hpp>

using dStorm::output::Output;

namespace dStorm {
namespace engine {

class Car::ActiveProducer {
    Car& c;
    boost::unique_lock< boost::mutex > lock;
  public:
    ActiveProducer( Car& c ) 
        : c(c), lock(c.ring_buffer_mutex)
    { 
        ++c.producer_count; 
        lock.unlock();
    }
    ~ActiveProducer() {
        lock.lock();
        --c.producer_count; 
        c.consumer_can_continue.notify_all();
    }
};

class Car::ComputationThread {
  private:
    boost::thread thread;
    Car& car;
    std::auto_ptr<ActiveProducer> producer;

  public:
    ComputationThread(Car &car) 
    : car(car), producer(new ActiveProducer(car) )
    { 
        thread = boost::thread( &ComputationThread::run, this );
    }

    DSTORM_REALIGN_STACK void run() {
        try {
            car.run_computation( producer, car.terminate_early );
        } catch ( const boost::exception& e ) {
            boost::lock_guard<boost::mutex> lock2( car.ring_buffer_mutex );
            car.error = boost::current_exception();
        } catch ( const std::runtime_error& e ) {
            boost::lock_guard<boost::mutex> lock2( car.ring_buffer_mutex );
            car.error = boost::copy_exception(e);
        }
    }

    ~ComputationThread() {
        DEBUG("Collecting piston");
        car.producer_can_continue.notify_all();
        thread.join(); 
    }
};

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

Car::Car (JobMaster* input_stream, const dStorm::GrandConfig &new_config) 
: simparm::Listener( simparm::Event::ValueChanged ),
  config(new_config),
  ident( getRunNumber() ),
  runtime_config("dStormJob" + ident, "dStorm Job " + ident),
  abortJob("StopComputation", "Stop computation"),
  closeJob("CloseJob", "Close job"),
  input(NULL),
  output(NULL),
  close_job( new_config.auto_terminate() ),
  terminate_early(false),
  repeat_run(false),
  blocked(false),
  ring_buffer(),
  producer_count(0)
{
    //DEBUG("Building car from config " << &config << " and meta info " << &(config.get_meta_info()) );
    used_output_filenames = config.get_meta_info().forbidden_filenames;
    closeJob.helpID = "#CloseJob";
    abortJob.helpID = "#StopEngine";

    receive_changes_from( abortJob.value );
    receive_changes_from( closeJob.value );
    receive_changes_from( runtime_config );

    //DEBUG("Determining input file name from basename " << config.get_meta_info().suggested_output_basename.new_basename());
    output::Basename bn( config.get_meta_info().suggested_output_basename );
    bn.set_variable("run", ident);
    //DEBUG("Setting output basename to " << bn.unformatted()() << " (expanded " << bn.new_basename() << ")");
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

    if ( job_handle.get() != NULL )
        job_handle->unregister_node();

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
        boost::lock_guard<boost::mutex> lock2( ring_buffer_mutex );
        DEBUG("Job close button allows termination" );
        close_job = true;
        consumer_can_continue.notify_all();
        terminate_early = true;
        producer_can_continue.notify_all();
    } else if ( &e.source == &abortJob.value && e.cause == simparm::Event::ValueChanged && abortJob.triggered() )
    {
        DEBUG("Abort job button pressed");
        abortJob.untrigger();
        abortJob.editable = false;
        boost::lock_guard<boost::mutex> lock2( ring_buffer_mutex );
        terminate_early = true;
        producer_can_continue.notify_all();
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

void Car::compute_until_terminated() {
    int number_of_threads = 1;
    if ( input->capabilities().test( input::BaseSource::ConcurrentIterators ) ) {
        number_of_threads = config.pistonCount();
        DEBUG("Using " << number_of_threads << " threads");
    } else {
        DEBUG("Multiple concurrent iterators not supported, using only a single thread.");
    }

    while (true) {
        {
            DEBUG("Announcing run");
            boost::lock_guard<boost::recursive_mutex> lock( mutex );
            Output::RunRequirements r = 
                output->announce_run(Output::RunAnnouncement());
            if ( ! r.test(Output::MayNeedRestart) )
                input->dispatch( Input::WillNeverRepeatAgain );
            next_output = first_output;
            boost::range::fill( ring_buffer, 
                boost::optional<output::LocalizedImage>() );
            repeat_run = false;
        }

        DEBUG("Adding computation threads");
        for (int i = 0; i < number_of_threads; ++i) 
            threads.push_back(  new ComputationThread(*this) );
        DEBUG("Running output thread");
        output_ring_buffer();
        DEBUG("Collecting threads");
        threads.clear();
        DEBUG("Collected threads");

        if ( error )
            boost::rethrow_exception(error);

        boost::lock_guard<boost::recursive_mutex> lock( mutex );
        if ( repeat_run ) {
            output->propagate_signal( Output::Engine_is_restarted );
            continue;
        } else {
            output->propagate_signal( Output::Engine_run_succeeded );
            break;
        }
    }
}

void Car::run_computation(std::auto_ptr<ActiveProducer>, bool& stop) 
{
    for (Input::iterator i = input->begin(), e = input->end(); i != e; ++i) 
    {
        const output::LocalizedImage& r = *i;
        boost::unique_lock<boost::mutex> lock(ring_buffer_mutex);
        DEBUG("Inserting output for " << i->forImage);
        while ( (r.forImage - next_output).value() >=
                int(ring_buffer.size()) ) 
        {
            if ( stop ) { DEBUG("Abnormal finish"); return; }
            producer_can_continue.wait(lock);
        }

        if ( stop ) { DEBUG("Abnormal finish"); return; }
        int ring = r.forImage.value() % ring_buffer.size();
        assert( ! ring_buffer[ring].is_initialized() );
        ring_buffer[ring] = r;
        if ( r.forImage == next_output )
            consumer_can_continue.notify_all();
    }
    DEBUG("Normal finish");
}

void Car::output_ring_buffer() {
    boost::unique_lock<boost::mutex> lock(ring_buffer_mutex);
    const int mod = ring_buffer.size();
    while ( true ) 
    {
        const int ring = next_output.value() % mod;
        while ( ! ring_buffer[ring].is_initialized() )
        {
            if ( producer_count == 0 ) return;
            consumer_can_continue.wait( lock );
        }
        while ( blocked ) { consumer_can_continue.wait( lock ); }

        lock.unlock();

        boost::unique_lock<boost::recursive_mutex> output_lock(mutex);
        assert( ring_buffer[ring].is_initialized() );
        output->receiveLocalizations( *ring_buffer[ring] );
        ring_buffer[ring].reset();
        output_lock.unlock();

        lock.lock();
        next_output += 1 * camera::frame;
        producer_can_continue.notify_all();
    }
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

    runtime_config.push_back( *input );
    runtime_config.push_back( output->getNode() );
    runtime_config.push_back( abortJob );
    runtime_config.push_back( closeJob );

    input::BaseSource::Wishes requirements;
    if ( config.pistonCount() > 1 )
        requirements.set( input::BaseSource::ConcurrentIterators );

    DEBUG("Getting input traits from " << input.get());
    Input::TraitsPtr traits = input->get_traits(requirements);
    first_output = next_output = *traits->image_number().range().first;
    DEBUG("Job length declared as " << traits->image_number().range().second.get_value_or( -1 * camera::frame ) );
    DEBUG("Creating announcement from traits " << traits.get());
    Output::Announcement announcement( *traits );
    upstream_engine = announcement.engine;
    announcement.engine = this;
    announcement.output_chain_mutex = &mutex;
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
        compute_until_terminated();
    }

    DEBUG("Erasing input " << input.get());
    input.reset(NULL);
    DEBUG("Erased input");
  } catch (const std::bad_alloc& e) {
    OutOfMemoryMessage m("Job " + ident);
    runtime_config.send(m);
  } catch (const std::runtime_error& e) {
    DEBUG("Sending message in drive mode");
    simparm::Message m( "Error in Job " + ident, e.what() );
    runtime_config.send(m);
    DEBUG("Sent message in drive mode");
  }

    boost::unique_lock<boost::recursive_mutex> lock( mutex );
    DEBUG("Waiting for termination allowance");
    while ( ! close_job )
        consumer_can_continue.wait(lock);
    DEBUG("Allowed to terminate");

    if ( config.configTarget ) {
        std::ostream& stream = config.configTarget.get_output_stream();
        std::list<std::string> lns = config.printValues();
        for (std::list<std::string>::const_iterator i = lns.begin(); i != lns.end(); i++)
            stream << *i << "\n";
        config.configTarget.close_output_stream();
    }
}

void Car::stop() {
    abortJob.trigger();
    closeJob.trigger();
}

void Car::restart() {
    repeat_run = true;
    terminate_early = true;
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

struct Car::Block : public EngineBlock {
    Car& m;
    Block( Car& m ) : m(m) {
        boost::unique_lock<boost::mutex> lock(m.ring_buffer_mutex);
        DEBUG( "Block in place" );
        m.blocked = true;
    }

    ~Block() {
        DEBUG( "Waiting for lock to put block out of place" );
        boost::unique_lock<boost::mutex> lock(m.ring_buffer_mutex);
        m.blocked = false;
        DEBUG( "Block out of place" );
        m.consumer_can_continue.notify_all();
    }
};

std::auto_ptr<EngineBlock> Car::block() {
    return std::auto_ptr<EngineBlock>( new Block(*this) );
}

}
}

