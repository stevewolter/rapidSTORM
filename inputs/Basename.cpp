#include "Basename.h"
#include <dStorm/input/Source_impl.h>
#include <dStorm/input/InputMutex.h>

namespace dStorm {
namespace input {
namespace Basename {

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
: input::chain::Filter(),
  simparm::Listener( simparm::Event::ValueChanged ),
  default_output_basename(""),
  user_changed_output(false)
{
    receive_changes_from( output.value );
}

ChainLink::ChainLink(const ChainLink& o) 
: input::chain::Filter(o),
  simparm::Listener( simparm::Event::ValueChanged ),
  traits(o.traits),
  default_output_basename(o.default_output_basename),
  user_changed_output(o.user_changed_output)
{
    receive_changes_from( output.value );
}

chain::Link::AtEnd ChainLink::traits_changed( TraitsRef traits, Link *l )
{
    Link::traits_changed(traits,l); 

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
        return notify_of_trait_change(this->traits);
    } else {
        output = default_output_basename;
        return notify_of_trait_change(traits);
    }
}

chain::Link::AtEnd ChainLink::context_changed( ContextRef context, Link *l )
{
    Link::context_changed(context,l); 
    return this->notify_of_context_change(context); 
}

void ChainLink::operator()(const simparm::Event&)
{
    ost::MutexLock lock( global_mutex() );
    if ( output() == "" ) output = default_output_basename;
    user_changed_output = ( output() != "" && output() != default_output_basename );
    if ( traits.get() ) {
        traits->suggested_output_basename.unformatted() = output();
        notify_of_trait_change( this->traits );
    }
}

std::auto_ptr<chain::Filter> makeLink() {
    return std::auto_ptr<chain::Filter>( new ChainLink() );
}

}
}
}
