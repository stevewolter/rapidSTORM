#include "Basename.h"
#include <dStorm/input/Source.h>
#include <dStorm/input/InputMutex.h>
#include <dStorm/signals/BasenameChange.h>
#include <dStorm/input/Forwarder.h>
#include <simparm/Entry.hh>
#include <simparm/Structure.hh>
#include <dStorm/input/MetaInfo.h>

namespace dStorm {
namespace basename_input_field {

using namespace input;

class Config : public simparm::Object {
  public:
    simparm::StringEntry output;

    Config();
    void registerNamedEntries();
};

class ChainLink 
: public Forwarder, public simparm::Listener , simparm::Structure<Config>
{
    MetaInfo::Ptr traits;
    std::string default_output_basename;
    bool user_changed_output;

  protected:
    void operator()(const simparm::Event&);

  public:
    ChainLink();
    ChainLink* clone() const { return new ChainLink(*this); }
    simparm::Node& getNode() { return static_cast<Config&>(*this); }
    void registerNamedEntries( simparm::Node& n ) {
        receive_changes_from( output.value );
        Forwarder::registerNamedEntries(n);
        n.push_back( *this );
    }
    std::string name() const { return getName(); }
    std::string description() const { return getDesc(); }

    void traits_changed( TraitsRef r, Link* l);

    BaseSource* makeSource() { return Forwarder::makeSource(); }
};

Config::Config()
: simparm::Object("OutputBasename", "Set output basename"),
  output("Basename", "Output file basename", "")
{
    output.helpID = "OutputBasename";
}

void Config::registerNamedEntries() {
    push_back( output );
}

ChainLink::ChainLink() 
: input::Forwarder(),
  simparm::Listener( simparm::Event::ValueChanged ),
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
        this->traits->suggested_output_basename.unformatted() = output();
        traits->get_signal< signals::BasenameChange >()( this->traits->suggested_output_basename );
    } else {
        output = default_output_basename;
    }
    /* Check that no recursive call triggered by a signal happened */
    if ( upstream_traits() == traits ) 
        update_current_meta_info(this->traits);
}

void ChainLink::operator()(const simparm::Event&)
{
    boost::lock_guard<boost::mutex> lock( global_mutex() );
    if ( output() == "" ) output = default_output_basename;
    user_changed_output = ( output() != "" && output() != default_output_basename );
    if ( traits.get() ) {
        traits->suggested_output_basename.unformatted() = output();
        traits->get_signal< signals::BasenameChange >()( traits->suggested_output_basename );
        update_current_meta_info( this->traits );
    }
}

std::auto_ptr<Link> makeLink() {
    return std::auto_ptr<Link>( new ChainLink() );
}

}
}
