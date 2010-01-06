#define DSTORM_CAR_CPP
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
#include <dStorm/input/LocalizationFileReader.h>
#include <CImg.h>
#include "doc/help/context.h"
#include <dStorm/engine/Input.h>

using namespace std;

namespace dStorm {
namespace engine {

ost::Mutex Car::terminationMutex;
ost::Condition Car::terminationChanged( terminationMutex );
bool Car::terminateAll = false;

/* ==== This code gives a new job ID on each call. === */
static ost::Mutex *runNumberMutex = NULL;
static char number[6];
static std::string getRunNumber() {
    if ( ! runNumberMutex) {
        runNumberMutex = new ost::Mutex();
        strcpy(number, "    0");
    }
    ost::MutexLock lock(*runNumberMutex);

    int index = 4;
    number[index]++;
    while (index > 0)
        if (number[index] == '9'+1) {
            number[index] = '0';
            number[--index]++;
        } else
            break;
    index = 4;
    while (index >= 0 && isdigit(number[index])) index--;
    return std::string(number+index+1);
}

Car::Car (const MasterConfig::Ptr& master, const CarConfig &new_config) 
: ost::Thread("Car"),
  master( master ),
  config(new_config),
  ident( getRunNumber() ),
  runtime_config("dStormJob" + ident, "dStorm Job " + ident),
  closeJob("CloseJob", "Close job"),
  input(NULL),
  output(NULL),
  terminate(false)
{
    PROGRESS("Building car");
    closeJob.helpID = HELP_CloseJob;

    receive_changes_from( closeJob.value );
    receive_changes_from( runtime_config );

    PROGRESS("Building output");
    output = config.outputSource.make_output();
    if ( output.get() == NULL )
        throw std::invalid_argument("No valid output supplied.");

    PROGRESS("Registering at master config");
    this->master->thread_safely_register_node( runtime_config );
}

Car::~Car() 
{
    PROGRESS("Destructing Car");
    PROGRESS("Joining car subthread");
    join();

    PROGRESS("Sending destruction signal to outputs");
    output->propagate_signal( output::Output::Prepare_destruction );

    PROGRESS("Removing from master config");
    /* Remove from simparm parents to hide destruction process
     * from interface. */
    master->thread_safely_erase_node( runtime_config );

    output.reset(NULL);
    locSource.reset(NULL);
    myEngine.reset(NULL);
}

void Car::operator()(simparm::Node& src, Cause c, simparm::Node*) {
    if ( &src == &closeJob.value && c == ValueChanged && closeJob.triggered() )
    {
        ost::MutexLock lock( terminationMutex );
        PROGRESS("Job close button allows termination");
        if ( myEngine.get() != NULL ) myEngine->stop();
        terminate = true;
        terminationChanged.signal();
    } else if ( &src == &runtime_config && c == RemovedParent ) {
        PROGRESS("Noticed parent removal");
        if ( ! runtime_config.isActive() ) {
            PROGRESS("Runtime config got inactive");
            ost::MutexLock lock( terminationMutex );
            if ( myEngine.get() != NULL )
                myEngine->stop();
            terminate = true;
            terminationChanged.signal();
        }
    }
}

void Car::run() throw() {
    try {
        drive();
    } catch ( const std::bad_alloc& e ) {
        std::cerr << "Your system has insufficient memory for this dSTORM job. "
                  << "The most common reason is a high resolution enhancement combined "
                  << "with a large image. Try reducing either the image size or the "
                  << "resolution enhancement.\n";
    } catch ( const std::exception& e ) {
        std::cerr << "dSTORM  job failed. Reason: " << e.what() << "\n";
        std::cerr.flush();
    }
}

void Car::drive() {
    PROGRESS("Determining type of input driver");
    try {
        std::auto_ptr<input::BaseSource> source;
        source = config.inputConfig.makeImageSource();

        if ( source->can_provide< Image >() ) {
            PROGRESS("Have image input, registering input");
            runtime_config.push_back( *source );
            PROGRESS("Making input buffer");
            input.reset(
                new Input( input::BaseSource::downcast<Image>(source) ) );
            PROGRESS("Making engine");
            myEngine.reset( 
                new Engine(config.engineConfig, *input, *output) );
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

    ost::MutexLock lock( terminationMutex );
    PROGRESS("Waiting for termination allowance");
    if ( runtime_config.isActive() )
        while ( ! terminate && ! terminateAll )
            terminationChanged.wait();
    PROGRESS("Allowed to terminate");

    /* TODO: We have to check here if the job was _really_ finished
    * successfully. */
    output->propagate_signal( output::Output::Job_finished_successfully );

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
    buncher.noteTraits( *locSource, 
                        config.inputConfig.firstImage(), 
                        config.inputConfig.lastImage() );
    locSource->startPushing( &buncher );
}

void Car::terminate_all_Car_threads() {
    Engine::stopAllEngines();
    terminateAll = true;
    terminationChanged.signal();
}

}
}

