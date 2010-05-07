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
#include <dStorm/localization_file/reader.h>

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

Car::Car (JobMaster* input_stream, const CarConfig &new_config) 
: ost::Thread("Car"),
  simparm::Listener( simparm::Event::ValueChanged ),
  input_stream( input_stream ),
  config(new_config),
  ident( getRunNumber() ),
  runtime_config("dStormJob" + ident, "dStorm Job " + ident),
  closeJob("CloseJob", "Close job"),
  input(NULL),
  output(NULL),
  terminate( new_config.auto_terminate() ),
  terminationChanged( terminationMutex )
{
    DEBUG("Building car");
    if ( config.inputConfig.inputMethod().uses_input_file() )
        used_output_filenames.insert( config.inputConfig.inputFile() );
    closeJob.helpID = HELP_CloseJob;

    receive_changes_from( closeJob.value );
    receive_changes_from( runtime_config );

    DEBUG("Determining input file name");
    output::Basename bn( config.inputConfig.basename() );
    bn.set_variable("run", ident);
    DEBUG("Setting output basename to " << bn.unformatted()() << " (expanded " << bn.new_basename() << ")");
    config.outputSource.set_output_file_basename( bn );
    DEBUG("Building output");
    output = config.outputSource.make_output();
    if ( output.get() == NULL )
        throw std::invalid_argument("No valid output supplied.");
    output->check_for_duplicate_filenames( used_output_filenames );

    DEBUG("Registering at input_stream config");
    if ( input_stream )
        this->input_stream->register_node( *this );
}

Car::~Car() 
{
    DEBUG("Destructing Car");
    DEBUG("Joining car subthread");
    join();

    DEBUG("Sending destruction signal to outputs");
    output->propagate_signal( output::Output::Prepare_destruction );

    DEBUG("Removing from input_stream config");
    /* Remove from simparm parents to hide destruction process
     * from interface. */
    if ( input_stream )
        input_stream->erase_node( *this );

    DEBUG("Deleting outputs");
    output.reset(NULL);
    DEBUG("Deleting localization source");
    locSource.reset(NULL);
    DEBUG("Deleting engine");
    myEngine.reset(NULL);
    DEBUG("Commencing destruction");
}

void Car::operator()(const simparm::Event& e) {
    if ( &e.source == &closeJob.value && e.cause == simparm::Event::ValueChanged && closeJob.triggered() )
    {
        closeJob.untrigger();
        closeJob.editable = false;
        ost::MutexLock lock( terminationMutex );
        DEBUG("Job close button allows termination");
        if ( myEngine.get() != NULL ) myEngine->stop();
        if ( locSource.get() != NULL ) locSource->stopPushing();
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
    DEBUG("Determining type of input driver");
    try {
        source = config.inputConfig.makeImageSource();

        if ( source->can_provide< Image >() ) {
            DEBUG("Have image input, registering input");
            runtime_config.push_back( *source );
            DEBUG("Making input buffer");
            input.reset(
                new Input( input::BaseSource::downcast<Image>(source) ) );
            DEBUG("Making engine");
            myEngine.reset( 
                new Engine(
                    config.engineConfig, ident,
                    *input, *output) );
            runtime_config.push_back( *myEngine );
        } else if ( source->can_provide< Localization >() ) {
            DEBUG("Have localization input");
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
    make_input_driver();

    runtime_config.push_back( *output );
    runtime_config.push_back( closeJob );

    DEBUG("Starting computation");
    if ( myEngine.get() != NULL ) {
        DEBUG("Computing with engine");
        myEngine->run();
    } else if (locSource.get() != NULL) {
        DEBUG("Computing from STM file");
        runOnSTM();
    }
    DEBUG("Ended computation");
    DEBUG("Erasing carburettor");
    input.reset(NULL);
    DEBUG("Erased carburettor");

    ost::MutexLock lock( terminationMutex );
    DEBUG("Waiting for termination allowance");
    while ( ! terminate )
        terminationChanged.wait();
    DEBUG("Allowed to terminate");

    /* TODO: We have to check here if the job was _really_ finished
    * successfully. */
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

void Car::runOnSTM() throw( std::exception ) {
    LOCKING("Running on STM file");
    LocalizationBuncher buncher(*output);
    LocalizationFile::Reader::Source *reader
        = dynamic_cast<LocalizationFile::Reader::Source*>( locSource.get() );
    if ( reader )
        reader->setEmptyImageCallback( &buncher );
    buncher.noteTraits( *locSource, 
                        config.inputConfig.firstImage() 
                            * cs_units::camera::frame, 
                        config.inputConfig.lastImage()
                            * cs_units::camera::frame );
    locSource->startPushing( &buncher );
    buncher.ensure_finished();
}

void Car::stop() {
    closeJob.trigger();
}

void Car::abnormal_termination(std::string r) throw() {
    std::cerr << "Job " << ident << " had a critical "
              << "error: " << r << " Terminating job."
              << std::endl;
}

}
}

