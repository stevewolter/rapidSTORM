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

//#undef PROGRESS
//#define PROGRESS(x) std::cerr << x << "\n";
//#undef STATUS
//#define STATUS(x) std::cerr << x << "\n";

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

Car::Car (const CarConfig &new_config) 
: ost::Thread("Car"),
  config(new_config),
  runtime_config("PRELIMINARY", "PRELIMINARY"),
  closeJob("CloseJob", "Close job"),
  input(NULL),
  output(NULL),
  terminate(false)
{
    STATUS("Building car");
    closeJob.helpID = HELP_CloseJob;

    /* Acquire a job descriptor */
    string runNumber = getRunNumber();
    runtime_config.setName(string("dStormJob") + runNumber);
    runtime_config.setDesc(string("dStorm Job ") + runNumber);

    receive_changes_from( closeJob );
    receive_changes_from( runtime_config );

    STATUS("Building output");
    output = config.outputSource.make_output();
    if ( output.get() == NULL )
        throw std::invalid_argument("No valid output supplied.");

    /* The configuration I/O is copied at the end of this routine
     * to include as many passenger contributions as possible. */
    STATUS("Copying configuration I/O");
    runtime_config.make_to_sibling_of(new_config);
}

Car::~Car() 
{
    STATUS("Destructing Car");
    join();
    PROGRESS("Joined car subthread");

    output->propagate_signal( output::Output::Prepare_destruction );
    PROGRESS("Finished destruction signal to transmissions");

    /* Remove from simparm parents to hide destruction process
     * from interface. */
    while ( ! runtime_config.getParents().empty() )
        runtime_config.getParents().front()->erase( runtime_config );
    PROGRESS("Removed all runtime_config parents");

    output.reset(NULL);
    PROGRESS("Erased transmission");
    locSource.reset(NULL);
    myEngine.reset(NULL);
    PROGRESS("Erased engine");
}

void Car::operator()(simparm::Node& src, Cause c, simparm::Node*) {
    if ( &src == &closeJob && c == ValueChanged && closeJob.triggered() )
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
    try {
        std::auto_ptr<input::BaseSource> source;
        source = config.inputConfig.makeImageSource();

        if ( source->can_provide< Image >() ) {
            input.reset(new Input( source->downcast<Image>(source)));
            myEngine.reset( 
                new Engine(config.engineConfig, *input, *output) );
        } else if ( source->can_provide< Localization >() ) {
            locSource = source->downcast<Localization>(source);
        } else
            throw std::runtime_error("No valid source specified.");
    } catch (const std::exception& e) {
        throw std::runtime_error(
            "Error in constructing input driver: " 
                + string(e.what()) );
    }

    if ( locSource.get() != NULL )
        runtime_config.push_back( *locSource );
    else if ( myEngine.get() != NULL) {
        runtime_config.push_back( input->getConfig() );
        runtime_config.push_back( *myEngine );
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

