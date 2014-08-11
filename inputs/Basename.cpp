#include "inputs/Basename.h"

#include "helpers/make_unique.hpp"
#include "input/Source.h"
#include "input/InputMutex.h"
#include "input/Forwarder.h"
#include "simparm/Entry.h"
#include "input/MetaInfo.h"

namespace dStorm {
namespace basename_input_field {

using namespace input;

class Config {
  public:
    simparm::StringEntry output;

    Config();
    void attach_ui( simparm::NodeHandle at ) { output.attach_ui( at ); }
};

template <typename Type>
class ChainLink 
: public Forwarder<Type>
{
    typedef typename Link<Type>::TraitsRef TraitsRef;

    simparm::Object name_object;
    Config config;
    MetaInfo::Ptr traits;
    std::string default_output_basename;
    bool user_changed_output;
    simparm::BaseAttribute::ConnectionStore listening;

    void republish_traits_locked();

  public:
    ChainLink(std::unique_ptr<input::Link<Type>> upstream);
    ChainLink* clone() const OVERRIDE { return new ChainLink(*this); }
    void registerNamedEntries( simparm::NodeHandle n ) OVERRIDE {
        Forwarder<Type>::registerNamedEntries(n);
        config.attach_ui( name_object.attach_ui( n ) );

        listening = config.output.value.notify_on_value_change( 
            boost::bind( &ChainLink::republish_traits_locked, this ) );
    }
    std::string name() const OVERRIDE { return name_object.getName(); }
    input::Source<Type>* makeSource() OVERRIDE { return this->upstream_source().release(); }

    void traits_changed( TraitsRef r, Link<Type>* l) OVERRIDE;
};

Config::Config()
: output("Basename", "")
{
}

template <typename Type>
ChainLink<Type>::ChainLink(std::unique_ptr<input::Link<Type>> upstream) 
: input::Forwarder<Type>(std::move(upstream)),
  name_object( "OutputBasename", "Set output basename" ),
  default_output_basename(""),
  user_changed_output(false) {
}

template <typename Type>
void ChainLink<Type>::traits_changed( TraitsRef traits, Link<Type> *l )
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
    } else {
        config.output = default_output_basename;
    }
    /* Check that no recursive call triggered by a signal happened */
    if ( this->upstream_traits() == traits ) 
        this->update_current_meta_info(this->traits);
}

template <typename Type>
void ChainLink<Type>::republish_traits_locked()
{
    input::InputMutexGuard lock( global_mutex() );
    if ( config.output() == "" ) config.output = default_output_basename;
    user_changed_output = ( config.output() != "" && config.output() != default_output_basename );
    if ( traits.get() ) {
        traits->suggested_output_basename.unformatted() = config.output();
        this->update_current_meta_info( this->traits );
    }
}

std::unique_ptr<input::Link<engine::ImageStack>> makeImageLink(
    std::unique_ptr<input::Link<engine::ImageStack>> upstream) {
    return make_unique<ChainLink<engine::ImageStack>>(std::move(upstream));
}

std::unique_ptr<input::Link<output::LocalizedImage>> makeLocalizationLink(
    std::unique_ptr<input::Link<output::LocalizedImage>> upstream) {
    return make_unique<ChainLink<output::LocalizedImage>>(std::move(upstream));
}

}
}
