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
  config(config),
  output_mutex(NULL),
  implementation( config.colourScheme.value().make_backend(this->config, *this) ),
  forwardOutput( &implementation->getForwardOutput() )
{
    DEBUG("Building viewer");

    histogramPower.helpID = "#Viewer_Status_Power";
    histogramPower.userLevel = simparm::Object::Beginner;
    tifFile.helpID = "#Viewer_Status_ToFile";
    save.helpID = "#Viewer_Status_Save";

    reshow_output.viewable = ! config.showOutput();

    push_back( histogramPower );
    push_back( tifFile );
    push_back( save );
    push_back( reshow_output );

    receive_changes_from( reshow_output.value );
    receive_changes_from( save.value );
    receive_changes_from( histogramPower.value );

    DEBUG("Built viewer");
}

Viewer::~Viewer() {
    DEBUG("Destructing Viewer");
}


void Viewer::receiveLocalizations(const EngineResult& er)
{
    forwardOutput->receiveLocalizations(er);
}

Output::AdditionalData 
Viewer::announceStormSize(const Announcement &a) {
    output_mutex = a.output_chain_mutex;
    implementation->set_output_mutex( output_mutex );
    implementation->set_job_name( a.description );
    this->manager = &a.display_manager();
    return forwardOutput->announceStormSize(a);
}

Viewer::RunRequirements Viewer::announce_run(const RunAnnouncement& a) {
    return forwardOutput->announce_run(a);
}

void Viewer::store_results() {
    forwardOutput->store_results();
    if (tifFile)
        writeToFile(tifFile());
}

void Viewer::operator()(const simparm::Event& e) {
    if ( ! output_mutex ) return;
    boost::lock_guard<boost::recursive_mutex> lock(*output_mutex);
    if (&e.source == &reshow_output.value && reshow_output.triggered()) {
        reshow_output.untrigger();
        config.showOutput = true;
        adapt_to_changed_config();
    } else if (&e.source == &save.value && save.triggered()) {
        /* Save image */
        save.untrigger();
        if ( tifFile ) {
            writeToFile( tifFile() );
        }
    } else if (&e.source == &histogramPower.value) {
        /* Change histogram power */
        implementation->set_histogram_power(histogramPower());
    } 
}

void Viewer::adapt_to_changed_config() {
    DEBUG("Changing implementation, showing output is " << config.showOutput());
    if ( implementation.get() ) {
        implementation = implementation->adapt( implementation, config, *this );
        implementation->set_output_mutex( output_mutex );
        forwardOutput = &implementation->getForwardOutput();
        reshow_output.viewable = ! config.showOutput();
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
    insert_filename_with_check( tifFile(), present_filenames );
}

std::auto_ptr<output::OutputSource> make_output_source() {
    return std::auto_ptr<output::OutputSource>( new output::FileOutputBuilder<Viewer>() );
}

}
}
