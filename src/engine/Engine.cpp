#define DSTORM_ENGINE_CPP

#include "engine/Engine.h"

#include <data-c++/Vector.h>
#include <cassert>
#include <errno.h>
#include <string.h>
#include <algorithm>
#include <foreach.h>

#include "engine/SigmaGuesser.h"
#include "engine/SpotFinder.h"
#include "engine/SpotFitter.h"
#include "engine/Image.h"
#include "engine/Config.h"
#include "engine/Input.h"
#include "transmissions/Crankshaft.h"
#include <sstream>
#include "engine/EngineDebug.h"
#include <CImgBuffer/Slot.h>
#include <CImgBuffer/Source.h>
#include <CImgBuffer/ImageTraits.h>
#include "help_context.h"

#ifdef DSTORM_MEASURE_TIMES
#include <time.h>
clock_t smooth_time = 0, search_time = 0, fit_time = 0;
#endif

using namespace dStorm;
using namespace std;
using namespace CImgBuffer;

namespace dStorm {

bool Engine::globalStop=false;

class EngineThread : public ost::Thread {
  private:
    Engine &engine;
    auto_ptr<string> nm;
  public:
    EngineThread(Engine &engine, auto_ptr<string> name) 
        : ost::Thread(name->c_str()), engine(engine), nm(name) {}
    ~EngineThread() {
        STATUS("Collecting piston");
        join(); 
    }
    void run() throw() {
        try {
            engine.runPiston();
        } catch (const std::bad_alloc& e) {
            cerr << PACKAGE_NAME << " ran out of memory. Try removing the filter "
                    "output module or reducing image resolution "
                    "enhancement." << endl;
            engine.emergencyStop = true;
        } catch (const std::exception& e) {
            cerr << PACKAGE_NAME << ": Error in computation: " << e.what() << endl;
            engine.emergencyStop = true;
        } catch (...) {
            cerr << PACKAGE_NAME << ": Unknown engine failure." << endl;
            engine.emergencyStop = true;
        }
    }
};

Engine::Engine(Config &config, Input &input, Output& output)
: Object("EngineStatus", "Computation status"),
  config(config),
  stopper("EngineStopper", "Stop computation"),
  input(input), output(&output), emergencyStop(false),
  error(false)
{
    STATUS("Constructing engine");

    stopper.helpID = HELP_StopEngine;
    receive_changes_from( stopper );
    push_back( config.sigma_x);
    push_back( config.sigma_y);
    push_back( config.sigma_xy);
    push_back( stopper );
}

void Engine::addPiston() {
    int pistonCount = pistons.size();
    auto_ptr<string> pistonName( new string("Piston 00") );
    (*pistonName)[7] += pistonCount / 10;
    (*pistonName)[8] += pistonCount % 10;
    auto_ptr<ost::Thread> new_piston(new EngineThread(*this, pistonName));
    STATUS("Spawning new piston");
    new_piston->start();
    pistons.push_back( new_piston );
}

void Engine::restart() {
    STATUS("Restarting");
    emergencyStop = false;
    error = false;
    STATUS("Cleaning input");
    input.makeAllUntouched();
    STATUS("Deleting all results");
    output->propagate_signal( Output::Engine_is_restarted );
    STATUS("Restarted");
}

void Engine::collectPistons() {
    pistons.clear();
}

void Engine::run() 
{
    STATUS("Started run");

    int numPistons = config.pistonCount();

    STATUS("Making SigmaGuesser");
    std::auto_ptr<Crankshaft> temporaryCrankshaft;
    if ( ! config.fixSigma() ) {
        Crankshaft *crankshaft = dynamic_cast<Crankshaft*>(output);
        std::auto_ptr<Output> guesser
            (new SigmaGuesserMean( config, input ));
        if ( crankshaft != NULL )  {
            crankshaft->push_front( guesser, Crankshaft::State );
        } else {
            temporaryCrankshaft.reset( new Crankshaft() );
            temporaryCrankshaft->add( guesser, Crankshaft::State );
            temporaryCrankshaft->add( *output );
            output = temporaryCrankshaft.get();
        }
    } else
        input.setDiscardingLicense( true );

    STATUS("Announcing size");
    const CImgBuffer::Traits<Image>& prop = input.getTraits();
    Output::Announcement announcement
        ( prop.dimx(), prop.dimy(), input.size() );
    STATUS("Built announcement structure");

    Output::AdditionalData data 
        = output->announceStormSize(announcement);
    if ( data & Output::LocalizationSources ) {
        cerr << "The engine module cannot provide localization traces."
             << "Please select an appropriate transmission.\n";
        return;
    }

    STATUS("Ready to fork " << numPistons << " computation threads");
#ifdef DSTORM_MEASURE_TIMES
    cerr << "Warning: Time measurement probes active" << endl;
    numPistons = 1;
#endif
    while (true) {
        for (int i = 0; i < numPistons-1; i++)
            addPiston();
        runPiston();

        STATUS("Collecting running pistons");
        collectPistons();
        STATUS("Collected pistons");

        if ( globalStop)
            break;
        else if (emergencyStop) {
            if (error) {
                output->propagate_signal( Output::Engine_run_failed );
                break;
            } else
                restart();
        } else {
            output->propagate_signal( Output::Engine_run_succeeded );
            break;
        }
    }
    STATUS("Finished run");
}

void Engine::runPiston() 
{
    int maximumLimit = 20;
    const CImgBuffer::Traits<Image>& imProp = input.getTraits();
    STATUS("Started piston");
    STATUS("Building spot finder with dimensions " << imProp.dimx() <<
           " " << imProp.dimy());
    auto_ptr<SpotFinder> finder
        = SpotFinder::factory(config, imProp.dimx(), imProp.dimy() );

    STATUS("Building spot fitter");
    auto_ptr<SpotFitter> fitter(SpotFitter::factory(config));

    STATUS("Building fit buffer");
    data_cpp::Vector<Localization> buffer(200, 0, Localization(-1,-1));

    STATUS("Building maximums");
    CandidateTree<SmoothedPixel> maximums
        (config.x_maskSize(), config.y_maskSize(), 1, 1);
    maximums.setLimit(maximumLimit);

    STATUS("Initialized maximums");
    int origMotivation = config.motivation();

    STATUS("Initialized motivation");
    Output::EngineResult resultStructure;
    resultStructure.smoothed = &finder->getSmoothedImage();
    resultStructure.candidates = &maximums;

    output->propagate_signal(Output::Engine_run_is_starting);

    foreach (i, Input, input) {
        STATUS("Intake (" << i->index() << ")");

        Claim< Image > claim = i->claim();
        if ( ! claim.isGood() ) continue;
        Image& image = *claim;

        PROGRESS("Compression (" << image.getImageNumber() << ")");
        IF_DSTORM_MEASURE_TIMES( clock_t prepre = clock() );
        finder->smooth(image);
        IF_DSTORM_MEASURE_TIMES( smooth_time += clock() - prepre );

        CandidateTree<SmoothedPixel>::iterator cM = maximums.begin();
        int motivation;
        recompress:  /* We jump here if maximum limit proves too small */
        IF_DSTORM_MEASURE_TIMES( clock_t pre = clock() );
        maximums.fill( finder->getSmoothedImage() );
        PROGRESS("Found " << maximums.size() << " spots");

        IF_DSTORM_MEASURE_TIMES( clock_t search_start = clock() );
        IF_DSTORM_MEASURE_TIMES( search_time += search_start - pre );

        /* Motivational fitting */
        motivation = origMotivation;
        for ( cM = maximums.begin(); cM.hasMore() && motivation > 0; cM++){
            LOCKING("Trying candidate");
            const Spot& s = cM->second;
            /* Get the next spot to fit and fit it. */
            Localization *candidate = buffer.allocate();
            int found_number = 
                fitter->fitSpot(s, image, claim.index(), candidate);
            if ( found_number > 0 ) {
                LOCKING("Good fit");
                motivation = origMotivation;
                buffer.commit(found_number);
            } else if ( found_number < 0 ) {
                LOCKING("Bad fit");
                motivation += found_number;
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
        resultStructure.forImage = claim.index();
        resultStructure.first = buffer.ptr();
        resultStructure.number = buffer.size();
        resultStructure.source = &image;

        Output::Result r = 
            output->receiveLocalizations( resultStructure );
        if (r == Output::RestartEngine || r == Output::StopEngine ) 
        {
            STATUS("Emergency stop: Engine restart requested");
            output->propagate_signal( Output::Engine_run_is_aborted );
            emergencyStop = true;
            error = ( r == Output::StopEngine );
        }

        PROGRESS("Exhaust");
        buffer.clear();

        if (emergencyStop || globalStop) {
            PROGRESS("Emergency stop");
            break;
        }
    }
    STATUS("Finished piston");
}

Engine::~Engine() {
    STATUS("Destructing engine");
    emergencyStop = true;
    collectPistons();
    STATUS("Destructed engine");
}

}
