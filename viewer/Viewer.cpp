#include "debug.h"
#include "viewer/plugin.h"
#include <stdint.h>
#include "viewer/Viewer.h"
#include <limits>
#include <cassert>
#include "engine/Image.h"

#include "simparm/Message.h"
#include "display/Manager.h"
#include <boost/thread/locks.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include "viewer/ColourScheme.h"

#include <fstream>

using namespace std;

using namespace dStorm::display;
using namespace dStorm::output;

namespace dStorm {
namespace viewer {

Viewer::Viewer(const Config& config)
: Status(config),
  repeater( NULL )
{
    DEBUG("Building viewer");

    this->config.backend_needs_changing( boost::bind( &Viewer::make_new_backend, this ) );

    DEBUG("Built viewer");
}

Viewer::~Viewer() {
    DEBUG("Destructing Viewer " << this);
}

void Viewer::attach_ui_( simparm::NodeHandle at ) {
    listening[0] = config.showOutput.value.notify_on_value_change( 
        boost::bind( &Viewer::adapt_to_changed_config, this ) );
    listening[1] = save.value.notify_on_value_change( 
        boost::bind( &Viewer::save_image, this ) );
    listening[2] = config.histogramPower.value.notify_on_value_change( 
        boost::bind( &Viewer::change_histogram_normalization_power, this ) );
    listening[3] = config.top_cutoff.value.notify_on_value_change( 
        boost::bind( &Viewer::change_top_cutoff, this ) );

    ui = at;
    Status::attach_ui( at );
}

void Viewer::receiveLocalizations(const EngineResult& er)
{
    boost::lock_guard<boost::mutex> lock(mutex);
    forwardOutput->receiveLocalizations(er);
}

Output::AdditionalData 
Viewer::announceStormSize(const Announcement &a) {
    boost::lock_guard<boost::mutex> lock(mutex);
    announcement = a;
    repeater = a.engine;
    this->engine = a.engine;
    implementation = Backend::create( this->config.colourScheme().make_backend( this->config.invert() ), *this);
    implementation->set_job_name( a.description );
    forwardOutput = &implementation->getForwardOutput();
    return forwardOutput->announceStormSize(a);
}

Viewer::RunRequirements Viewer::announce_run(const RunAnnouncement& a) {
    boost::lock_guard<boost::mutex> lock(mutex);
    return forwardOutput->announce_run(a);
}

void Viewer::store_results_( bool job_successful ) {
    boost::lock_guard<boost::mutex> lock(mutex);
    if ( forwardOutput )
        forwardOutput->store_results(job_successful);
    if (job_successful && config.outputFile)
        writeToFile(config.outputFile());
}

void Viewer::make_new_backend() {
    if ( announcement ) {
            if ( repeater && repeater->can_repeat_results() ) {
                /* Store the old implementation past the mutex lock to allow mutex 
                * locking in the course of the destructor. This is needed when the
                * live backend is destructed because a last update is fetched by
                * the display thread. */
                std::auto_ptr<Backend> behind_the_scenes( new NoOpBackend() );
                {
                    boost::lock_guard<boost::mutex> lock(mutex);
                    std::swap( behind_the_scenes, implementation );
                    forwardOutput = &implementation->getForwardOutput();
                }
                behind_the_scenes.reset();
                behind_the_scenes = Backend::create( this->config.colourScheme().make_backend(this->config.invert()), *this);
                behind_the_scenes->set_job_name( announcement->description );
                behind_the_scenes->getForwardOutput().announceStormSize(*announcement);
                boost::lock_guard<boost::mutex> lock(mutex);
                std::swap( behind_the_scenes, implementation );
                forwardOutput = &implementation->getForwardOutput();
                repeater->repeat_results();
            } else {
                simparm::Message m("Cannot change display parameters without cache",
                    "Changing the display parameters has no effect without a Cache output.",
                    simparm::Message::Warning);
            }
        }
}

void Viewer::save_image() {
    if ( save.triggered() ) {
        boost::lock_guard<boost::mutex> lock(mutex);
        /* Save image */
        save.untrigger();
        if ( config.outputFile ) {
            writeToFile( config.outputFile() );
        }
    }
}

void Viewer::change_histogram_normalization_power() {
    boost::lock_guard<boost::mutex> lock(mutex);
    implementation->set_histogram_power(config.histogramPower());
}

void Viewer::change_top_cutoff() {
    boost::lock_guard<boost::mutex> lock(mutex);
    implementation->set_top_cutoff(config.top_cutoff());
}

void Viewer::adapt_to_changed_config() {
    DEBUG("Changing implementation, showing output is " << config.showOutput());
    std::auto_ptr<Backend> backend;
    if ( implementation.get() ) {
        /* The backends are swapped under the mutex here, but
         * destruction is delayed until the mutex is given up.
         * Otherwise, the live backend would lock up when it tries
         * to give up its display under the mutex. */
        boost::lock_guard<boost::mutex> lock(mutex);
        backend = implementation->change_liveness( *this );
        if ( backend.get() ) {
            std::swap( backend, implementation );
            forwardOutput = &implementation->getForwardOutput();
        }
    }
    DEBUG("Changed implementation to " << implementation.get());
}

void Viewer::writeToFile(const string &name) {
    try {
        implementation->save_image(name, config);
    } catch ( const std::runtime_error& e ) {
        simparm::Message m( "Writing result image failed", "Writing result image failed: " + std::string(e.what()) );
        m.send( ui );
    }
}

void Viewer::check_for_duplicate_filenames
        (std::set<std::string>& present_filenames)
{
    insert_filename_with_check( config.outputFile(), present_filenames );
}

std::auto_ptr<output::OutputSource> make_output_source() {
    return std::auto_ptr<output::OutputSource>( new output::FileOutputBuilder<Config,Viewer>() );
}

}
}
