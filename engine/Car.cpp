#define DSTORM_CAR_CPP
#include "debug.h"

#include "Car.h"
#include "LocalizationBuncher.h"
#include <dStorm/input/ImageTraits.h>
#include <dStorm/input/Source.h>
#include <dStorm/output/Localizations.h>
#include "Engine.h"
#include <dStorm/engine/Image.h>
#include <dStorm/input/Config.h>
#include <fstream>
#include <queue>
#include <dStorm/output/Output.h>
#include <CImg.h>
#include "doc/help/context.h"
#include <dStorm/engine/Input.h>
#include <dStorm/input/Method.h>

using namespace std;

namespace dStorm {
namespace engine {

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

Car::Car (JobMaster& input_stream, const CarConfig &new_config) 
: ost::Thread("Car"),
  simparm::Listener( simparm::Event::ValueChanged ),
  input_stream( input_stream ),
  config(new_config),
  ident( getRunNumber() ),
  runtime_config("dStormJob" + ident, "dStorm Job " + ident),
  closeJob("CloseJob", "Close job"),
  input(NULL),
  output(NULL),
  terminate(false),
  terminationChanged( terminationMutex ),
  panic_point_set( false )
{
    PROGRESS("Building car");
    if ( config.inputConfig.inputMethod().uses_input_file() )
        used_output_filenames.insert( config.inputConfig.inputFile() );
    closeJob.helpID = HELP_CloseJob;

    receive_changes_from( closeJob.value );
    receive_changes_from( runtime_config );

    PROGRESS("Determining input file name");
    output::Basename bn( config.inputConfig.basename() );
    bn.set_variable("run", ident);
    config.outputSource.set_output_file_basename( bn );
    PROGRESS("Building output");
    output = config.outputSource.make_output();
    if ( output.get() == NULL )
        throw std::invalid_argument("No valid output supplied.");
    output->check_for_duplicate_filenames( used_output_filenames );

    PROGRESS("Registering at input_stream config");
    this->input_stream.register_node( *this );
}

Car::~Car() 
{
    PROGRESS("Destructing Car");
    PROGRESS("Joining car subthread");
    join();

    PROGRESS("Sending destruction signal to outputs");
    output->propagate_signal( output::Output::Prepare_destruction );

    PROGRESS("Removing from input_stream config");
    /* Remove from simparm parents to hide destruction process
     * from interface. */
    input_stream.erase_node( *this );

    output.reset(NULL);
    locSource.reset(NULL);
    myEngine.reset(NULL);
}

void Car::operator()(const simparm::Event& e) {
    if ( &e.source == &closeJob.value && e.cause == simparm::Event::ValueChanged && closeJob.triggered() )
    {
        ost::MutexLock lock( terminationMutex );
        PROGRESS("Job close button allows termination");
        if ( myEngine.get() != NULL ) myEngine->stop();
        terminate = true;
        terminationChanged.signal();
    }
}

void Car::run() throw() {
    try {
        drive();
    } catch ( const std::bad_alloc& e ) {
        std::cerr << "Your system has insufficient memory for job "
                  << ident << ". "
                  << "The most common reason is a high resolution enhancement combined "
                  << "with a large image. Try reducing either the image size or the "
                  << "resolution enhancement.\n";
    } catch ( const std::exception& e ) {
        std::cerr << "Job " << ident << " failed. Reason: " << e.what() << "\n";
        std::cerr.flush();
    }
}

void Car::make_input_driver() {
    PROGRESS("Determining type of input driver");
    try {
        source = config.inputConfig.makeImageSource();

        if ( source->can_provide< Image >() ) {
            PROGRESS("Have image input, registering input");
            runtime_config.push_back( *source );
            PROGRESS("Making input buffer");
            input.reset(
                new Input( input::BaseSource::downcast<Image>(source) ) );
            PROGRESS("Making engine");
            myEngine.reset( 
                new Engine(
                    config.engineConfig, ident,
                    *input, *output) );
            runtime_config.push_back( *myEngine );
        } else if ( source->can_provide< Localization >() ) {
            PROGRESS("Have localization input");
            locSource = source->downcast<Localization>(source);
            runtime_config.push_back( *locSource );
        } else
            throw std::runtime_error("No valid source specified.");
    } catch (const std::exception& e) {
        throw std::runtime_error(
            "Error in constructing input driver: " 
                + string(e.what()) );
    }
}

void Car::drive() {
    bool successful_finish = true;
    if ( setjmp( panic_point ) == 0 ) {
        panic_point_set = true;
        make_input_driver();

        runtime_config.push_back( *output );
        runtime_config.push_back( closeJob );

        PROGRESS("Starting computation");
        if ( myEngine.get() != NULL ) {
            myEngine->run();
        } else if (locSource.get() != NULL) {
            runOnSTM();
        }
        PROGRESS("Ended computation");
        PROGRESS("Erasing carburettor");
        input.reset(NULL);
        PROGRESS("Erased carburettor");
    } else {
        /* Abnormal termination after received signal. */
        successful_finish = false;
    }
    panic_point_set = false;

    if ( setjmp(panic_point) == 0 ) {
        panic_point_set = true;
        ost::MutexLock lock( terminationMutex );
        PROGRESS("Waiting for termination allowance");
        if ( runtime_config.isActive() )
            while ( ! terminate )
                terminationChanged.wait();
        PROGRESS("Allowed to terminate");

        /* TODO: We have to check here if the job was _really_ finished
        * successfully. */
        if ( successful_finish )
            output->propagate_signal( 
                output::Output::Job_finished_successfully );

        if ( config.configTarget ) {
            std::ostream& stream = config.configTarget.get_output_stream();
            list<string> lns = config.printValues();
            for (list<string>::const_iterator i = lns.begin(); i != lns.end(); i++)
                stream << *i << "\n";
            config.configTarget.close_output_stream();
        }
    }
}

void Car::runOnSTM() throw( std::exception ) {
    LOCKING("Running on STM file");
    LocalizationBuncher buncher(*output);
    buncher.noteTraits( *locSource, 
                        config.inputConfig.firstImage() 
                            * cs_units::camera::frame, 
                        config.inputConfig.lastImage()
                            * cs_units::camera::frame );
    locSource->startPushing( &buncher );
}

void Car::stop() {
    closeJob.trigger();
}

void Car::abnormal_termination(std::string r) throw() {
    if ( panic_point_set ) {
        std::cerr << "Job " << ident << " had a critical "
                  << "error: " << r << " Terminating job."
                  << std::endl;
        longjmp( panic_point, 1 );
    } else        
        Runnable::abnormal_termination(r);
}

}
}

