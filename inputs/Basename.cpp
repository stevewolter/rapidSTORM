#include "Basename.h"
#include <dStorm/input/Source.h>
#include <dStorm/input/InputMutex.h>
#include <dStorm/signals/BasenameChange.h>
#include <dStorm/input/Forwarder.h>
#include <simparm/Entry.hh>
#include <dStorm/input/MetaInfo.h>

namespace dStorm {
namespace basename_input_field {

using namespace input;

class Config {
  public:
    simparm::StringEntry output;

    Config();
    void attach_ui( simparm::Node& at ) { output.attach_ui( at ); }
};

class ChainLink 
: public Forwarder
{
    simparm::Object name_object;
    Config config;
    MetaInfo::Ptr traits;
    std::string default_output_basename;
    bool user_changed_output;
    simparm::BaseAttribute::ConnectionStore listening;

    void republish_traits_locked();

  public:
    ChainLink();
    ChainLink* clone() const { return new ChainLink(*this); }
    void registerNamedEntries( simparm::Node& n ) {
        Forwarder::registerNamedEntries(n);
        config.attach_ui( name_object.attach_ui( n ) );

        listening = config.output.value.notify_on_value_change( 
            boost::bind( &ChainLink::republish_traits_locked, this ) );
    }
    std::string name() const { return name_object.getName(); }
    std::string description() const { return name_object.getDesc(); }

    void traits_changed( TraitsRef r, Link* l);

    BaseSource* makeSource() { return Forwarder::makeSource(); }
};

Config::Config()
: output("Basename", "Output file basename", "")
{
    output.helpID = "OutputBasename";
}

ChainLink::ChainLink() 
: input::Forwarder(),
  name_object( "OutputBasename", "Set output basename" ),
  default_output_basename(""),
  user_changed_output(false)
{
}

void ChainLink::traits_changed( TraitsRef traits, Link *l )
{
    if ( traits.get() == NULL )  {
        default_output_basename = "";
        this->traits.reset();
    } else {
        this->traits.reset( traits->clone() );
        if ( traits->suggested_output_basename.unformatted()() != "" )
            default_output_basename = traits->suggested_output_basename.unformatted()();
    }

    if ( user_changed_output ) {
        if ( this->traits.get() )
            this->traits->suggested_output_basename.unformatted() = config.output();
        if ( traits.get() )
            traits->get_signal< signals::BasenameChange >()( this->traits->suggested_output_basename );
    } else {
        config.output = default_output_basename;
    }
    /* Check that no recursive call triggered by a signal happened */
    if ( upstream_traits() == traits ) 
        update_current_meta_info(this->traits);
}

void ChainLink::republish_traits_locked()
{
    input::InputMutexGuard lock( global_mutex() );
    if ( config.output() == "" ) config.output = default_output_basename;
    user_changed_output = ( config.output() != "" && config.output() != default_output_basename );
    if ( traits.get() ) {
        traits->suggested_output_basename.unformatted() = config.output();
        traits->get_signal< signals::BasenameChange >()( traits->suggested_output_basename );
        update_current_meta_info( this->traits );
    }
}

std::auto_ptr<Link> makeLink() {
    return std::auto_ptr<Link>( new ChainLink() );
}

}
}
