#include "Config.h"
#include "doc/help/context.h"
#include "OutputSource.h"
#include <simparm/ChoiceEntry_Impl.hh>

#include "debug.h"

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
    } catch (const std::exception& e) {}
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
    registerNamedEntries();
}

ChoiceConfig::ChoiceConfig( const ChoiceConfig& other )
: simparm::Object(other),
  std::auto_ptr<OutputSource>( other.get()->clone() )
{
    registerNamedEntries();
}

ChoiceConfig::~ChoiceConfig()
{
    DEBUG("Choice config destructor");
}

Config::Config() 
: simparm::NodeChoiceEntry<ChoiceConfig>
    ("ChooseTransmission", "Choose new output"),
  SourceFactory( static_cast<simparm::Node&>(*this) ),
  simparm::Node::Callback( Node::ValueChanged )
{
    DEBUG("Constructing output config from scratch");
    set_auto_selection( false );
    this->helpID = HELP_ChooseOutput;

    receive_changes_from( value );
}

Config::Config( const Config& other ) 
: simparm::NodeChoiceEntry<ChoiceConfig>(other, 
        simparm::NodeChoiceEntry<ChoiceConfig>::DeepCopy ),
  SourceFactory( static_cast<simparm::Node&>(*this), other),
  simparm::Node::Callback( Node::ValueChanged ),
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

    for ( iterator i = beginChoices(); i != endChoices(); i++) {
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
        std::auto_ptr<OutputSource> rv( value().get()->clone() );
        rv->set_output_factory( *this );
        rv->set_source_capabilities( my_capabilities );
        return rv;
    } else {
        return std::auto_ptr<OutputSource>(NULL);
    }
}

void Config::addChoice(OutputSource *toAdd) {
    toAdd->set_source_capabilities( my_capabilities );
    simparm::NodeChoiceEntry<ChoiceConfig>::addChoice(
        new ChoiceConfig( std::auto_ptr<OutputSource>(toAdd) ) );
}

void Config::operator()(Node& src, Cause c, Node* arg)
{
    if ( &src == &value ) {
        notifyChangeCallbacks( c, arg );
    }
}

}
}
