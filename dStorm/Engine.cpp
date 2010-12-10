#define DSTORM_ENGINE_CPP

#include "Engine.h"

#include <data-c++/Vector.h>
#include <cassert>
#include <errno.h>
#include <string.h>
#include <algorithm>
#include <foreach.h>

#include "SigmaGuesser.h"
#include "SpotFinder.h"
#include "SpotFitter.h"
#include "Image.h"
#include "Config.h"
#include "Carburettor.h"
#include "Crankshaft.h"
#include <sstream>
#include "EngineDebug.h"
#include <CImgBuffer/Slot.h>

#ifdef DSTORM_MEASURE_TIMES
#include <time.h>
clock_t smooth_time = 0, search_time = 0, fit_time = 0;
#endif

using namespace dStorm;
using namespace std;
using namespace cimg_library;
using namespace CImgBuffer;

namespace dStorm {

class EngineThread : public ost::Thread {
  private:
    Engine &engine;
    auto_ptr<string> nm;
  public:
    EngineThread(Engine &engine, auto_ptr<string> name) 
        : ost::Thread(name->c_str()), engine(engine), nm(name) {}
    ~EngineThread() throw() { join(); }
    void run() throw() { engine.safeRunPiston(); }
};

Engine::Engine(Config &config, Carburettor &input)
: ConfigObject("EngineStatus", "Computation status"),
  config(config),
  input(input), output(output), emergencyStop(false),
  error(false)
{
    push_back( config.sigma_x);
    push_back( config.sigma_y);
    push_back( config.sigma_xy);
}

void Engine::addPiston() throw() {
    int pistonCount = pistons.size();
    auto_ptr<string> pistonName( new string("Piston 00") );
    (*pistonName)[7] += pistonCount / 10;
    (*pistonName)[8] += pistonCount % 10;
    auto_ptr<ost::Thread> new_piston(new EngineThread(*this, pistonName));
    STATUS("Spawning new piston");
    new_piston->start();
    pistons.push_back( new_piston );
}

void Engine::safeRunPiston() throw() {
    try {
        runPiston();
    } catch (const std::exception& e) {
        cerr << PACKAGE_NAME << ": Engine failure: " << e.what() << endl;
        emergencyStop = true;
    }
}

void Engine::restart() throw() {
    STATUS("Restarting");
    emergencyStop = false;
    error = false;
    STATUS("Cleaning input");
    input.makeAllUntouched();
    STATUS("Deleting all results");
    output->deleteAllResults();
    STATUS("Restarted");
}

void Engine::collectPistons() throw() {
    pistons.clear();
}

void Engine::run() throw() 
{
    STATUS("Started run");

    int numPistons = config.pistonCount();

    STATUS("Making SigmaGuesser");
    std::auto_ptr<Crankshaft> temporaryCrankshaft;
    if ( ! config.fixSigma() ) {
        Crankshaft *crankshaft = dynamic_cast<Crankshaft*>(output);
        std::auto_ptr<Transmission> guesser(new SigmaGuesserMean( config, input ));
        if ( crankshaft != NULL )  {
            crankshaft->add( guesser, Crankshaft::State );
        } else {
            temporaryCrankshaft.reset( new Crankshaft() );
            temporaryCrankshaft->add( guesser, Crankshaft::State );
            temporaryCrankshaft->add( *output );
            output = temporaryCrankshaft.get();
        }
    }

    STATUS("Announcing size");
    output->announceStormSize( input.dimx(), input.dimy(), input.size() );

    STATUS("Ready to fork");
#ifdef DSTORM_MEASURE_TIMES
    cerr << "Measuring times" << endl;
    numPistons = 1;
#endif
    while (true) {
        for (int i = 0; i < numPistons-1; i++)
            addPiston();
        safeRunPiston();

        collectPistons();

        if (emergencyStop) {
            if (error) {
                output->deleteAllResults();
                break;
            } else
                restart();
        } else {
            output->finish();
            break;
        }
    }
    STATUS("Finished run");
}

void Engine::runPiston() throw(std::exception) 
{
    int maximumLimit = 20;
    STATUS("Started piston");
    STATUS("Building spot finder with dimensions " << input.dimx() <<
           " " << input.dimy());
    auto_ptr<SpotFinder> finder
        = SpotFinder::factory(config, input.dimx(), input.dimy() );
    STATUS("Building spot fitter");
    auto_ptr<SpotFitter> fitter(SpotFitter::factory(config));
    STATUS("Building fit buffer");
    data_cpp::Vector<Localization> buffer(200, 0, Localization(-1,-1));
    STATUS("Building maximums");
    CandidateTree<SmoothedPixel> maximums(config);
    maximums.setLimit(maximumLimit);
    STATUS("Initialized maximums");
    int origMotivation = config.motivation();
    STATUS("Initialized motivation");
    EngineResult resultStructure;
    resultStructure.carburettor = &input;
    resultStructure.smoothed = &finder->getSmoothed();
    resultStructure.candidates = &maximums;

    foreach (i, Carburettor, input) {
        STATUS("Intake (" << abs(i-input.begin()) << ")");
        Claim< Image > claim = i->claim();
        if ( ! claim.isGood() ) continue;
        Image& image = *claim;

        PROGRESS("Compression (" << image.getImageNumber() << ")");
        IF_DSTORM_MEASURE_TIMES( clock_t prepre = clock() );
        finder->prepare(image);
        IF_DSTORM_MEASURE_TIMES( smooth_time += clock() - prepre );
        fitter->prepareToFitImage(image);

        CandidateTree<SmoothedPixel>::iterator cM = maximums.begin();
        int motivation;
        recompress:  /* We jump here if maximum limit proves too small */
        IF_DSTORM_MEASURE_TIMES( clock_t pre = clock() );
        finder->findSpots(image, maximums);
        PROGRESS("Found " << maximums.size() << " spots");

        IF_DSTORM_MEASURE_TIMES( clock_t search_start = clock() );
        IF_DSTORM_MEASURE_TIMES( search_time += search_start - pre );

        /* Motivational fitting */
        motivation = origMotivation;
        for ( cM = maximums.begin(); cM.hasMore() && motivation > 0; cM++){
            const Spot& s = cM->second;
            /* Get the next spot to fit and fit it. */
            if ( fitter->fitSpot(s, buffer.allocate()) ) {
                motivation = origMotivation;
                if ( buffer.back().getStrength() <= 
                                config.amplitude_limit() )
                    buffer.commit();
            } else {
                motivation--;
            }
        }
        if (motivation > 0 && cM.limitReached()) {
            maximumLimit *= 2;
            PROGRESS("Raising maximumLimit to " << maximumLimit);
            maximums.setLimit(maximumLimit);
            buffer.clear();
            goto recompress;
        }

        PROGRESS("Found " << buffer.size() << " localizations");
        IF_DSTORM_MEASURE_TIMES( fit_time += clock() - search_start );

        PROGRESS("Power");
        resultStructure.forImage = image.getImageNumber();
        resultStructure.first = buffer.ptr();
        resultStructure.number = buffer.size();
        resultStructure.source = &image;

        Transmission::Result r = 
            output->receiveLocalizations( resultStructure );
        if (r == Transmission::RestartEngine ) {
            STATUS("Emergency stop: Engine restart requested");
            emergencyStop = true;
        } else if (r == Transmission::StopEngine) {
            STATUS("Emergency stop: Engine stop requested");
            emergencyStop = true;
            error = true;
        }

        PROGRESS("Exhaust");
        buffer.clear();

        if (emergencyStop) {
            PROGRESS("Emergency stop");
            return;
        }
    }
    STATUS("Finished piston");
}

Engine::~Engine() throw() {
    STATUS("Destructing engine");
    emergencyStop = true;
    collectPistons();
    STATUS("Destructed engine");
}

}
