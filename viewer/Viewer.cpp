#include "debug.h"
#include "plugin.h"
#include <stdint.h>
#include "Viewer.h"
#include <limits>
#include <cassert>
#include <dStorm/engine/Image.h>
#include <dStorm/doc/context.h>

#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Message.hh>
#include <dStorm/helpers/DisplayManager.h>

#include "ColourScheme.h"
#include "colour_schemes/hot_config.h"

using namespace std;
using namespace ost;

using namespace dStorm::Display;
using namespace dStorm::outputs;
using namespace dStorm::output;

namespace dStorm {
namespace outputs {

void add_viewer( output::Config& config ) {
    config.addChoice( new viewer::Viewer::Source() );
}

}
}

namespace dStorm {
namespace viewer {

Viewer::Viewer(const Viewer::Config& config)
: Status(config),
  OutputObject("Display", "Display status"),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  config(config),
  implementation( config.colourScheme.value().make_backend(this->config, *this) ),
  forwardOutput( &implementation->getForwardOutput() )
{
    DEBUG("Building viewer");

    histogramPower.helpID = HELP_Viewer_Status_Power;
    tifFile.helpID = HELP_Viewer_Status_ToFile;
    save.helpID = HELP_Viewer_Status_Save;

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


Output::Result
Viewer::receiveLocalizations(const EngineResult& er)
{
    MutexLock lock(implementation_mutex);
    return forwardOutput->receiveLocalizations(er);
}

Output::AdditionalData 
Viewer::announceStormSize(const Announcement &a) {
    MutexLock lock(implementation_mutex);
    return forwardOutput->announceStormSize(a);
}

void Viewer::propagate_signal(ProgressSignal s) {
    {
        MutexLock lock(implementation_mutex);
        forwardOutput->propagate_signal(s);
    }

    if (s == Engine_run_succeeded && tifFile) {
        writeToFile(tifFile());
    }
}

void Viewer::operator()(const simparm::Event& e) {
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
        MutexLock lock(implementation_mutex);
        /* Change histogram power */
        implementation->set_histogram_power(histogramPower());
    } 
}

void Viewer::adapt_to_changed_config() {
    DEBUG("Changing implementation, showing output is " << config.showOutput());
    MutexLock lock(implementation_mutex);
    if ( implementation.get() ) {
        implementation = implementation->adapt( implementation, config, *this );
        forwardOutput = &implementation->getForwardOutput();
        reshow_output.viewable = ! config.showOutput();
    }
    DEBUG("Changed implementation to " << implementation.get());
}

void Viewer::writeToFile(const string &name) {
    try {
        MutexLock lock(implementation_mutex);

        implementation->save_image(name, config);
    } catch ( const std::exception& e ) {
        simparm::Message m( "Writing result image failed", "Writing result image failed: " + std::string(e.what()) );
        send( m );
    }
}

void Viewer::check_for_duplicate_filenames
        (std::set<std::string>& present_filenames)
{
    insert_filename_with_check( tifFile(), present_filenames );
}

}
}
