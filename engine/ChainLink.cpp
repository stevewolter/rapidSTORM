#include "debug.h"

#include "ChainLink.h"
#include "Engine.h"
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/input/chain/Context_impl.h>
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
  my_context(c.my_context), my_traits(c.my_traits), config(c.config)
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

ChainLink::AtEnd
ChainLink::context_changed(ContextRef r, Link* l)
{
    Link::context_changed(r, l);

    my_context.reset( r->clone() );
    typedef input::Traits<dStorm::engine::Image> ImageTraits;
    if ( ! my_context->has_info_for<engine::Image>() )
        my_context->more_infos.push_back( new ImageTraits() );
    assert( my_context->has_info_for<engine::Image>() );

    make_new_requirements();
    Link::context_changed(my_context, l);
    return notify_of_context_change( my_context );
}

std::auto_ptr<input::chain::Filter>
make_rapidSTORM_engine_link()
{
    return std::auto_ptr<input::chain::Filter>( new ChainLink( ) );
}

void ChainLink::make_new_requirements() {
    input::Traits<dStorm::engine::Image>& reqs = 
        my_context->get_info_for<engine::Image>();
    reqs = input::Traits<dStorm::engine::Image>();
}

void ChainLink::operator()( const simparm::Event& e ) {
    ost::MutexLock lock( input::global_mutex() );
    if ( &e.source == &config.spotFindingMethod.value ) {
        make_new_requirements();
        notify_of_context_change( my_context );
    } else if ( &e.source == &config.spotFittingMethod.value ) {
        make_new_requirements();
        notify_of_context_change( my_context );
    } else if ( &e.source == &config.amplitude_threshold.value ) {
        if ( my_traits.get() )
            my_traits->suggested_output_basename.set_variable
                ( "thres", amplitude_threshold_string() );
        make_new_requirements();
        DEBUG("Basename is now " << my_traits->suggested_output_basename.new_basename() );
        notify_of_trait_change( my_traits );
        notify_of_context_change( my_context );
    } else {
        /* This else is called because some spot finder indicated that a
         * value changed. */
        make_new_traits();
    }
}

}
}
