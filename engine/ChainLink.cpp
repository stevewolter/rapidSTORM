#include "debug.h"

#include "ChainLink.h"
#include "Engine.h"
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/output/LocalizedImage_traits.h>
#include <boost/units/io.hpp>
#include <dStorm/input/InputMutex.h>

namespace dStorm {
namespace engine {

ChainLink::ChainLink() 
: simparm::Listener( simparm::Event::ValueChanged )
{
    receive_changes_from( config.amplitude_threshold.value );
    receive_changes_from( config.spotFittingMethod.value );
    receive_changes_from( config.spotFindingMethod.value );
}

ChainLink::ChainLink(const ChainLink& c)
: ClassicEngine(c),
  simparm::Listener( simparm::Event::ValueChanged ),
  my_traits(c.my_traits), config(c.config)
{
    DEBUG("Traits were copied from " << c.my_traits.get() << "," << c.current_traits().get() << " to "
            << my_traits.get() << "," << current_traits().get())
    if ( my_traits.get() != NULL )
        DEBUG("Basename after copying chain link is " << my_traits->suggested_output_basename << " or "
              << my_traits->suggested_output_basename << " in my own traits");
    receive_changes_from( config.amplitude_threshold.value );
    receive_changes_from( config.spotFittingMethod.value );
    receive_changes_from( config.spotFindingMethod.value );

    for ( simparm::NodeChoiceEntry< spot_fitter::Factory >::iterator 
            i = config.spotFittingMethod.beginChoices(); 
            i != config.spotFittingMethod.endChoices(); ++i )
    {
        i->register_trait_changing_nodes(*this);
    }
}

input::Source<output::LocalizedImage>*
ChainLink::makeSource()
{
    std::auto_ptr<input::BaseSource> base( Forwarder::makeSource() );
    if ( base->can_provide<engine::Image>() )
        return new Engine( config, input::BaseSource::downcast<engine::Image>(base) );
    else
        throw std::runtime_error("rapidSTORM engine cannot process the provided input");
}

std::string ChainLink::amplitude_threshold_string() const
{
    std::stringstream ss; 
    if ( config.amplitude_threshold().is_initialized() )
        ss << *config.amplitude_threshold();
    else 
        ss << "auto";
    return ss.str();
}

ChainLink::AtEnd
ChainLink::traits_changed(TraitsRef r, Link* l)
{
    Link::traits_changed(r, l);
    upstream_traits = r;
    make_new_traits();
    return AtEnd();
}

void ChainLink::make_new_traits() {
    if ( upstream_traits.get() == NULL ) {
        notify_of_trait_change( upstream_traits );
    } else {
        if ( ! upstream_traits->provides< engine::Image>() ) {
            my_traits.reset();
            notify_of_trait_change(my_traits);
            return;
        }

        boost::shared_ptr< input::Traits<output::LocalizedImage> >
            rt = Engine::convert_traits(config, upstream_traits->traits<engine::Image>());

        my_traits.reset( upstream_traits->clone() );
        my_traits->set_traits(rt);
        DEBUG("Setting traits variable thres for traits " << my_traits.get() << " and basename " << &my_traits->suggested_output_basename);
        my_traits->suggested_output_basename.set_variable
            ( "thres", amplitude_threshold_string() );
        DEBUG("Basename is now " << my_traits->suggested_output_basename << " for my traits " << my_traits.get() );
        notify_of_trait_change(my_traits);
        DEBUG("Finished notifying, traits are " << my_traits.get() << " " << current_traits().get());
    }
}

std::auto_ptr<input::chain::Link>
make_rapidSTORM_engine_link()
{
    return std::auto_ptr<input::chain::Link>( new ChainLink( ) );
}

void ChainLink::operator()( const simparm::Event& e ) {
    ost::MutexLock lock( input::global_mutex() );
    if ( &e.source == &config.spotFindingMethod.value ) {
        /* TODO: Only a context was published here. Are traits needed? */
    } else if ( &e.source == &config.spotFittingMethod.value ) {
        /* TODO: Only a context was published here. Are traits needed? */
    } else if ( &e.source == &config.amplitude_threshold.value ) {
        if ( my_traits.get() )
            my_traits->suggested_output_basename.set_variable
                ( "thres", amplitude_threshold_string() );
        DEBUG("Basename is now " << my_traits->suggested_output_basename.new_basename() );
        notify_of_trait_change( my_traits );
    } else {
        /* This else is called because some spot finder indicated that a
         * value changed. */
        make_new_traits();
    }
}

}
}
