#include "debug.h"
#include "plugin.h"
#include <stdint.h>
#include "Viewer.h"
#include <limits>
#include <cassert>
#include <dStorm/engine/Image.h>

#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Message.hh>
#include <dStorm/display/Manager.h>
#include <boost/thread/locks.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include "ColourScheme.h"
#include "colour_schemes/hot_config.h"

using namespace std;

using namespace dStorm::display;
using namespace dStorm::outputs;
using namespace dStorm::output;

namespace dStorm {
namespace viewer {

Viewer::Viewer(const Viewer::Config& config)
: Status(config),
  OutputObject("Display", "Display status"),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  repeater( NULL )
{
    DEBUG("Building viewer");

    this->registerNamedEntries( *this );
    Status::add_listener( *this );

    DEBUG("Built viewer");
}

Viewer::~Viewer() {
    DEBUG("Destructing Viewer " << this);
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
    this->manager = &a.display_manager();
    implementation = config.colourScheme.value().make_backend(this->config, *this);
    implementation->set_job_name( a.description );
    forwardOutput = &implementation->getForwardOutput();
    return forwardOutput->announceStormSize(a);
}

Viewer::RunRequirements Viewer::announce_run(const RunAnnouncement& a) {
    boost::lock_guard<boost::mutex> lock(mutex);
    return forwardOutput->announce_run(a);
}

void Viewer::store_results() {
    boost::lock_guard<boost::mutex> lock(mutex);
    forwardOutput->store_results();
    if (config.outputFile)
        writeToFile(config.outputFile());
}

void Viewer::operator()(const simparm::Event& e) {
    if ( &e.source == &config.showOutput.value ) {
        adapt_to_changed_config();
    } else if (&e.source == &save.value) {
        if ( save.triggered() ) {
            boost::lock_guard<boost::mutex> lock(mutex);
            /* Save image */
            save.untrigger();
            if ( config.outputFile ) {
                writeToFile( config.outputFile() );
            }
        }
    } else if (&e.source == &config.histogramPower.value) {
        /* Change histogram power */
        boost::lock_guard<boost::mutex> lock(mutex);
        implementation->set_histogram_power(config.histogramPower());
    } else if ( announcement ) {
        if ( repeater && repeater->can_repeat_results() ) {
            /* Store the old implementation past the mutex lock to allow mutex 
            * locking in the course of the destructor. This is needed when the
            * live backend is destructed because a last update is fetched by
            * the display thread. */
            std::auto_ptr<Backend> old_implementation;
            {
                boost::lock_guard<boost::mutex> lock(mutex);
                old_implementation = implementation;
                implementation = config.colourScheme.value().make_backend(this->config, *this);
                implementation->set_job_name( announcement->description );
                forwardOutput = &implementation->getForwardOutput();
                forwardOutput->announceStormSize(*announcement);
                repeater->repeat_results();
            }
            old_implementation.reset();
        } else {
            simparm::Message m("Cannot change display parameters without cache",
                "Changing the display parameters has no effect without a Cache output.",
                simparm::Message::Warning);
        }
    }
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
        send( m );
    }
}

void Viewer::check_for_duplicate_filenames
        (std::set<std::string>& present_filenames)
{
    insert_filename_with_check( config.outputFile(), present_filenames );
}

std::auto_ptr<output::OutputSource> make_output_source() {
    return std::auto_ptr<output::OutputSource>( new output::FileOutputBuilder<Viewer>() );
}

}
}
