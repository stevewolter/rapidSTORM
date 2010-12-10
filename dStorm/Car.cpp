#define DSTORM_CAR_CPP
#include "Car.h"
#include "Carburettor.h"
#include "Localizations.h"
#include "Engine.h"
#include "foreach.h"
#include "Image.h"
#include <CImgBuffer/Config.h>
#include <CImgBuffer/Image.h>
#include <fstream>
#include <queue>

using namespace std;

namespace dStorm {

/* ==== This code gives a new job ID on each call. === */
static ost::Mutex *runNumberMutex = NULL;
static char number[6];
static std::string getRunNumber() throw() {
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

Car::Car (const CarConfig &new_config,
          Carburettor *new_input, Transmission* given_output) throw (std::exception) 
: ost::Thread("Car"),
  config(new_config),
  runtime_config("PRELIMINARY", "PRELIMINARY"),
  myCarburettor(NULL),
  input(new_input),
  myTransmission( (given_output) ? NULL : 
                config.transmissionConfig.makeTransmission().release() ),
  output( (given_output) ? given_output : myTransmission.get() )
{
    STATUS("Building car");
    /* Acquire a job descriptor */
    string runNumber = getRunNumber();
    runtime_config.setName(string("dStormJob") + runNumber);
    runtime_config.setDesc(string("dStorm Job ") + runNumber);

    /* Register standard passengers. */
    STATUS("Building carburettor");
    if ( input == NULL ) {
        try {
            myCarburettor.reset( new Carburettor(config.inputConfig) );
            input = myCarburettor.get();
        } catch (const std::exception& e) {
            std::cerr << "Error in constructing input driver: " <<
                         e.what() << std::endl;
        }
    }

    if ( input == NULL ) { 
        throw std::invalid_argument("No valid input supplied.");
    }
    if ( output == NULL ) {
        cout << config.transmissionConfig.define() << "\n";
        throw std::invalid_argument("No valid output supplied.");
    }

    if (input->dimx() != 0) {
        myEngine.reset( new Engine(config.engineConfig, *input) );
        myEngine->setOutput( *output );
    }

    /* The configuration I/O is copied at the end of this routine
     * to include as many passenger contributions as possible. */
    STATUS("Copying configuration I/O");
    runtime_config.make_to_sibling_of(new_config);

    runtime_config.push_back( input->getConfig() );
    if ( myEngine.get() != NULL)
        runtime_config.push_back( *myEngine );
    runtime_config.push_back( *output );

}

Car::~Car() throw() 
{
    STATUS("Destructing Car");
    join();

    myCarburettor.reset(NULL);
    myTransmission.reset(NULL);
}

void Car::run() throw() {
    if ( myEngine.get() == NULL ) {
        runOnSTM();
    } else {
        myEngine->run();
    }

    if ( config.configTarget ) {
        std::ostream& stream = config.configTarget.getOutputStream();
        list<string> lns = config.printValues();
        for (list<string>::const_iterator i = lns.begin(); i != lns.end(); i++)
            stream << *i << "\n";
    }

    if ( config.printUsedSigma() ) {
        cout << "Sigmas were X " << config.engineConfig.sigma_x() <<
                ", Y " << config.engineConfig.sigma_y() <<
                ", XY " << config.engineConfig.sigma_xy() <<
                endl;
    }
}

void Car::runOnSTM() throw() {
    LOCKING("Running on STM file");
    try {
        istream &input = config.inputConfig.inputFile.getInputStream();

        Localizations::read( input, *output, 
            config.inputConfig.firstImage(), 
            config.inputConfig.lastImage(),
            config.engineConfig.amplitude_threshold() );
    } catch (const std::exception& e) {
        cerr << e.what() << endl;
    }
}


void Car::keepCarburettor() throw() {
    assert(myCarburettor.get() != input);
    myCarburettor.reset( input );
}
void Car::keepTransmission() throw() {
    assert(myTransmission.get() != output);
    myTransmission.reset( output );
}

}

