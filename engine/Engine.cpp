#define DSTORM_ENGINE_CPP

#include "debug.h"

#include "engine/Engine.h"
#include "config.h"
#include "ThresholdGuesser.h"

#include <dStorm/data-c++/Vector.h>
#include <cassert>
#include <errno.h>
#include <string.h>
#include <algorithm>

#include <dStorm/helpers/OutOfMemory.h>
#include "engine/SigmaGuesser.h"
#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFitter.h>
#include <dStorm/engine/Image.h>
#include <dStorm/engine/Config.h>
#include <dStorm/engine/Input.h>
#include <dStorm/engine/Input_decl.h>
#include <dStorm/outputs/Crankshaft.h>
#include <sstream>
#include "engine/EngineDebug.h"
#include <dStorm/input/Slot.h>
#include <dStorm/input/Source.h>
#include <dStorm/ImageTraits.h>
#include "doc/help/context.h"
#include <boost/units/io.hpp>
#include <dStorm/error_handler.h>
#include <dStorm/engine/SpotFitterFactory.h>
#include <dStorm/helpers/exception.h>

#ifdef DSTORM_MEASURE_TIMES
#include <time.h>
clock_t smooth_time = 0, search_time = 0, fit_time = 0;
#endif

using namespace dStorm;
using namespace std;
using namespace dStorm::input;
using namespace dStorm::output;
using namespace dStorm::outputs;

namespace dStorm {
namespace engine {

bool Engine::globalStop=false;

class EngineThread : public ost::Thread {
  private:
    Engine &engine;
    auto_ptr<string> nm;

  public:
    EngineThread(Engine &engine, 
                 auto_ptr<string> name) 
        : ost::Thread(name->c_str()), 
          engine(engine), nm(name) {}
    ~EngineThread() {
        DEBUG("Collecting piston");
        join(); 
    }
    void run() throw() {
        engine.safeRunPiston();
    }
    void abnormal_termination(std::string r) {
        std::cerr << "Computation thread " << *nm 
                  << " in job " << engine.job_ident
                  << " had a critical error: " << r
                  << " Terminating job computation."
                  << std::endl;
        engine.stop();
    }
};

Engine::Engine(
    Config &config, 
    std::string job_ident,
    Input &input, 
    Output& output
)
: Object("EngineStatus", "Computation status"),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  config(config),
  stopper("EngineStopper", "Stop computation"),
  errors("ErrorCount", "Number of dropped images", 0),
  input(input), output(&output), emergencyStop(false),
  error(false),
  job_ident( job_ident )
{
    DEBUG("Constructing engine");
    DEBUG("Spot fitter is named " << this->config.spotFittingMethod().getNode().getName());

    errors.editable = false;
    errors.viewable = false;

    stopper.helpID = HELP_StopEngine;
    receive_changes_from( stopper.value );
    push_back( config.sigma_x);
    push_back( config.sigma_y);
    push_back( config.sigma_xy);
    push_back( stopper );
    push_back( errors );
}

void Engine::addPiston() {
    int pistonCount = pistons.size();
    auto_ptr<string> pistonName( new string("Piston 00") );
    (*pistonName)[7] += pistonCount / 10;
    (*pistonName)[8] += pistonCount % 10;
    auto_ptr<ost::Thread> new_piston(new EngineThread(*this, pistonName));
    DEBUG("Spawning new piston");
    new_piston->start();
    pistons.push_back( new_piston );
}

void Engine::restart() {
    DEBUG("Restarting");
    emergencyStop = false;
    error = false;
    DEBUG("Cleaning input");
    input.dispatch( Input::RepeatInput );
    DEBUG("Resetting error count");
    errors = 0;
    DEBUG("Deleting all results");
    output->propagate_signal( Output::Engine_is_restarted );
    DEBUG("Restarted");
}

void Engine::collectPistons() {
    pistons.clear();
}

output::Traits Engine::convert_traits( std::auto_ptr<InputTraits> in ) {
    DEBUG("Getting other traits dimensionality");
    output::Traits rv( 
        in->get_other_dimensionality<Localization::Dim>() );
    DEBUG("Getting minimum amplitude");
    rv.min_amplitude = config.amplitude_threshold();
    rv.speed = in->speed;
    rv.first_frame = in->first_frame;
    rv.last_frame = in->last_frame;
    DEBUG("Last frame is set in input: " << in->last_frame.is_set());
    DEBUG("Last frame is set: " << rv.last_frame.is_set());
    DEBUG("Setting traits from spot fitter");
    config.spotFittingMethod().set_traits( rv );
    DEBUG("Returning traits");
    return rv;
}

void Engine::run() 
{
  try {
    DEBUG("Started run");

    int numPistons = config.pistonCount();

    DEBUG("Making SigmaGuesser");
    std::auto_ptr<Crankshaft> temporaryCrankshaft;
    if ( ! config.fixSigma() ) {
        Crankshaft *crankshaft = dynamic_cast<Crankshaft*>(output);
        std::auto_ptr<Output> guesser (new SigmaGuesserMean( config ));
        if ( crankshaft != NULL )  {
            crankshaft->push_front( guesser, Crankshaft::State );
        } else {
            temporaryCrankshaft.reset( new Crankshaft() );
            temporaryCrankshaft->add( guesser, Crankshaft::State );
            temporaryCrankshaft->add( *output );
            output = temporaryCrankshaft.get();
        }
    }

    DEBUG("Announcing size");
    Output::Announcement announcement(convert_traits(input.get_traits()));
    announcement.carburettor = &input;
    DEBUG("Built announcement structure");

    DEBUG("Guessing input threshold");
    if ( ! config.amplitude_threshold().is_set() )
        config.amplitude_threshold = ThresholdGuesser(input).compute_threshold();

    Output::AdditionalData data 
        = output->announceStormSize(announcement);
    if ( data.test( output::Capabilities::ClustersWithSources ) ) {
        simparm::Message m("Unable to provide data",
                "The engine module cannot provide localization traces."
                "Please select an appropriate transmission.");
        this->send(m);
        return;
    }

    DEBUG("Ready to fork " << numPistons << " computation threads");
#ifdef DSTORM_MEASURE_TIMES
    cerr << "Warning: Time measurement probes active" << endl;
    numPistons = 1;
#endif
    while (true) {
        Output::RunRequirements r = 
            output->announce_run(Output::RunAnnouncement());
        if ( ! r.test(Output::MayNeedRestart) )
            input.dispatch( Input::WillNeverRepeatAgain );

        for (int i = 0; i < numPistons-1; i++)
            addPiston();
        safeRunPiston();

        DEBUG("Collecting running pistons");
        collectPistons();
        DEBUG("Collected pistons");

        DEBUG("Checking stoppage for " << this << ": " << emergencyStop << " " << error << " " << globalStop << " " << dStorm::ErrorHandler::global_termination_flag());
        if (emergencyStop) {
            if (error || globalStop || 
                dStorm::ErrorHandler::global_termination_flag() ) 
            {
                output->propagate_signal( Output::Engine_run_failed );
                break;
            } else
                restart();
        } else {
            output->propagate_signal( Output::Engine_run_succeeded );
            break;
        }
    }
    DEBUG("Finished run");
  } catch (const dStorm::abort&) {
  } catch (const std::bad_alloc& e) {
    OutOfMemoryMessage m("Job " + job_ident);
    send(m);
  } catch (const dStorm::runtime_error& e) {
    simparm::Message m( e.get_message("Error in Job " + job_ident) );
    send(m);
  } catch (const std::exception& e) {
    simparm::Message m("Error in Job " + job_ident, e.what() );
    send(m);
  } catch (...) {
    simparm::Message m("Unspecified error", "Unknown type of failure. Sorry." );
    send( m );
  }
}

void Engine::safeRunPiston() throw()
{
    try {
        runPiston();
        return;
    } catch (const dStorm::abort&) {
    } catch (const std::bad_alloc& e) {
        OutOfMemoryMessage m("Job " + job_ident);
        send(m);
    } catch ( const dStorm::runtime_error& e ) {
        simparm::Message m( e.get_message("Error in Job " + job_ident) );
        send(m);
    } catch (const std::exception& e) {
        simparm::Message m("Error in Job " + job_ident, e.what() );
        send(m);
    } catch (...) {
        simparm::Message m("Unspecified error", "Unknown type of failure. Sorry." );
        send( m );
    }
    emergencyStop = error = true;
}

void Engine::runPiston() 
{
    int maximumLimit = 20;
    std::auto_ptr<InputTraits> imProp = input.get_traits();
    DEBUG("Started piston");
    DEBUG("Building spot finder with dimensions " << imProp->dimx() <<
           " " << imProp->dimy());
    if ( ! config.spotFindingMethod.isValid() )
        throw std::runtime_error("No spot finding method selected.");
    auto_ptr<SpotFinder> finder
        = config.spotFindingMethod().make_SpotFinder(config, imProp->size);

    DEBUG("Building spot fitter");
    auto_ptr<SpotFitter> fitter(config.spotFittingMethod().make_by_parts(config, *imProp));

    DEBUG("Building fit buffer");
    data_cpp::Vector<Localization> buffer;

    DEBUG("Building maximums");
    CandidateTree<SmoothedPixel> maximums
        (config.x_maskSize() / cs_units::camera::pixel,
         config.y_maskSize() / cs_units::camera::pixel,
         1, 1);
    maximums.setLimit(maximumLimit);

    DEBUG("Initialized maximums");
    int origMotivation = config.motivation();

    DEBUG("Initialized motivation");
    Output::EngineResult resultStructure;
    resultStructure.smoothed = &finder->getSmoothedImage();
    resultStructure.candidates = &maximums;

    output->propagate_signal(Output::Engine_run_is_starting);

    for (Source<Image>::iterator i = input.begin(); i != input.end(); i++)
    {
        DEBUG("Intake (" << i->frame_number() << ")");

        Image& image = *i;
        if ( image.is_invalid() ) {
            errors = errors() + 1;
            errors.viewable = true;
            continue;
        } else {
            DEBUG("Image " << i->ptr() << " is valid");
        }

        DEBUG("Compression (" << i->frame_number() << ")");
        IF_DSTORM_MEASURE_TIMES( clock_t prepre = clock() );
        finder->smooth(image);
        IF_DSTORM_MEASURE_TIMES( smooth_time += clock() - prepre );

        CandidateTree<SmoothedPixel>::iterator cM = maximums.begin();
        int motivation;
        recompress:  /* We jump here if maximum limit proves too small */
        IF_DSTORM_MEASURE_TIMES( clock_t pre = clock() );
        finder->findCandidates( maximums );
        DEBUG("Found " << maximums.size() << " spots");

        IF_DSTORM_MEASURE_TIMES( clock_t search_start = clock() );
        IF_DSTORM_MEASURE_TIMES( search_time += search_start - pre );

        /* Motivational fitting */
        motivation = origMotivation;
        for ( cM = maximums.begin(); cM.hasMore() && motivation > 0; cM++){
            const Spot& s = cM->second;
            DEBUG("Trying candidate at " << s.x() << "," << s.y() );
            /* Get the next spot to fit and fit it. */
            Localization *candidate = buffer.allocate();
            int found_number = fitter->fitSpot(s, image, candidate);
            if ( found_number > 0 ) {
                DEBUG("Good fit");
                for (int j = 0; j < found_number; j++)
                    candidate[j].setImageNumber( i->frame_number() );
                motivation = origMotivation;
                buffer.commit(found_number);
            } else if ( found_number < 0 ) {
                DEBUG("Bad fit");
                motivation += found_number;
            }
        }
        if (motivation > 0 && cM.limitReached()) {
            maximumLimit *= 2;
            DEBUG("Raising maximumLimit to " << maximumLimit);
            maximums.setLimit(maximumLimit);
            buffer.clear();
            goto recompress;
        }

        DEBUG("Found " << buffer.size() << " localizations");
        IF_DSTORM_MEASURE_TIMES( fit_time += clock() - search_start );

        DEBUG("Power");
        resultStructure.forImage = i->frame_number();
        resultStructure.first = buffer.ptr();
        resultStructure.number = buffer.size();
        resultStructure.source = &image;

        Output::Result r = 
            output->receiveLocalizations( resultStructure );
        if (r == Output::RestartEngine || r == Output::StopEngine ) 
        {
            DEBUG("Emergency stop: Engine restart requested");
            output->propagate_signal( Output::Engine_run_is_aborted );
            emergencyStop = true;
            if ( r == Output::StopEngine )
                error = true;
        }

        DEBUG("Exhaust");
        buffer.clear();

        DEBUG("Finished with image " << image.ptr());
        DEBUG("Checking for termination: " << emergencyStop << " " << globalStop << " " << ErrorHandler::global_termination_flag());
        if (emergencyStop || globalStop 
            || ErrorHandler::global_termination_flag()) 
        {
            DEBUG("Emergency stop");
            break;
        } else {
            DEBUG("Continuing");
        }
    }
    DEBUG("Finished piston");
}

Engine::~Engine() {
    DEBUG("Destructing engine");
    emergencyStop = true;
    collectPistons();
    DEBUG("Destructed engine");
}

}
}
