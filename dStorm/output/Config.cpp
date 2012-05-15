#include "debug.h"

#include "Config.h"
#include "OutputSource.h"
#include <simparm/ChoiceEntry_Impl.hh>

namespace dStorm {
namespace output {

std::string ChoiceConfig::getName() const {
    return get()->getNode().getName();
}

static void register_node_with_same_name_from_child(
    simparm::Node& p, simparm::Node& pn, simparm::Node& c
) {
    try {
        simparm::Node& cn = c[ pn.getName() ];
        p.erase( pn );
        p.push_back( cn );
    } catch (const std::logic_error& e) {}
}

void ChoiceConfig::registerNamedEntries()
{
    simparm::Node& n = get()->getNode();
    register_node_with_same_name_from_child( *this, desc, n );
    register_node_with_same_name_from_child( *this, viewable, n );
    register_node_with_same_name_from_child( *this, userLevel, n );
}

ChoiceConfig::ChoiceConfig( std::auto_ptr<OutputSource> src )
: simparm::Object(src->getNode().getName(), "Undefined description"),
  std::auto_ptr<OutputSource>( src )
{
    DEBUG("Choice config constructed " << this->get() << " named " << this->getNode().getName());
    registerNamedEntries();
}

ChoiceConfig::ChoiceConfig( const ChoiceConfig& other )
: simparm::Object(other),
  std::auto_ptr<OutputSource>( other.get()->clone() )
{
    DEBUG("Choice config copied " << this->get() << " named " << this->getNode().getName());
    registerNamedEntries();
}

ChoiceConfig::~ChoiceConfig()
{
    DEBUG("Choice config destructor for " << this << 
          " destructing source " << this->get() );
    this->reset();
    DEBUG("Destructed");
}

Config::Config() 
: simparm::ManagedChoiceEntry<ChoiceConfig>
    ("ChooseTransmission", "Choose new output"),
  SourceFactory( static_cast<simparm::Node&>(*this) ),
  simparm::Node::Callback( simparm::Event::ValueChanged )
{
    DEBUG("Constructing output config from scratch");
    set_auto_selection( false );
    this->helpID = "#ChooseOutput";

    receive_changes_from( value );
}

Config::Config( const Config& other ) 
: simparm::ManagedChoiceEntry<ChoiceConfig>(other),
  SourceFactory( static_cast<simparm::Node&>(*this), other),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  my_capabilities( other.my_capabilities )
{
    DEBUG("Copying output config");
    receive_changes_from( value );
}

Config* Config::clone() const {
    DEBUG("Cloning output config");
    return new Config(*this);
}

void Config::set_source_capabilities(Capabilities cap) {
    DEBUG("Notifying sources of new capabilities");
    my_capabilities = cap;

    for ( iterator i = begin(); i != end(); i++) {
        i->get()->set_source_capabilities( cap );
    }
    DEBUG("Notified sources of new capabilities");
}

Config::~Config() {
    DEBUG("Destructor");
}

std::auto_ptr<OutputSource> 
Config::make_output_source() 
{
    if ( isValid() ) {
        std::auto_ptr<OutputSource> rv( active_choice()->clone() );
        rv->set_output_factory( *this );
        rv->set_source_capabilities( my_capabilities );
        return rv;
    } else {
        return std::auto_ptr<OutputSource>(NULL);
    }
}

void Config::addChoice(OutputSource *toAdd) {
    toAdd->set_source_capabilities( my_capabilities );
    simparm::ManagedChoiceEntry<ChoiceConfig>::addChoice(
        std::auto_ptr<ChoiceConfig>(new ChoiceConfig( std::auto_ptr<OutputSource>(toAdd) ) ) );
}

void Config::operator()(const simparm::Event& e)
{
    if ( &e.source == &value ) {
        notifyChangeCallbacks( e.cause );
    }
}

}
}
