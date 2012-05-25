#include "debug.h"

#include "Config.h"
#include "OutputSource.h"
#include <simparm/ChoiceEntry_Impl.h>

namespace dStorm {
namespace output {

Config::Config() 
: choice("ChooseTransmission", "Choose new output")
{
    DEBUG("Constructing output config from scratch");
    choice.set_auto_selection( false );
    choice.setHelpID( "#ChooseOutput" );
}

Config* Config::clone() const {
    DEBUG("Cloning output config");
    return new Config(*this);
}

void Config::set_source_capabilities(Capabilities cap) {
    DEBUG("Notifying sources of new capabilities");
    my_capabilities = cap;

    for ( simparm::ManagedChoiceEntry<OutputSource>::iterator i = choice.begin(); i != choice.end(); i++) {
        i->set_source_capabilities( cap );
    }
    DEBUG("Notified sources of new capabilities");
}

Config::~Config() {
    DEBUG("Destructor");
}

void Config::attach_ui( simparm::NodeHandle at ) {
    listening = choice.value.notify_on_value_change( source_available );
    choice.attach_ui( at );
}

std::auto_ptr<OutputSource> 
Config::make_output_source() 
{
    if ( choice.isValid() ) {
        std::auto_ptr<OutputSource> rv( choice().clone() );
        rv->set_output_factory( *this );
        rv->set_source_capabilities( my_capabilities );
        choice.value = "";
        return rv;
    } else {
        return std::auto_ptr<OutputSource>(NULL);
    }
}

void Config::addChoice(OutputSource *toAdd) {
    toAdd->set_source_capabilities( my_capabilities );
    choice.addChoice( std::auto_ptr<OutputSource>(toAdd) );
}

void Config::notify_when_output_source_is_available( const Callback& cb ) {
    source_available.connect( cb );
}

}
}
