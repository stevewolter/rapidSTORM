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
    if ( config.amplitude_threshold().is_set() )
        ss << *config.amplitude_threshold();
    else 
        ss << "auto";
    return ss.str();
}

ChainLink::AtEnd
ChainLink::traits_changed(TraitsRef r, Link* l)
{
    Link::traits_changed(r, l);
    if ( r.get() == NULL ) return notify_of_trait_change( r );
    if ( ! r->provides< engine::Image>() ) {
        if ( my_context->throw_errors ) {
            throw std::runtime_error("rapidSTORM engine cannot process data of type " + r->base_traits().desc());
        } else {
            my_traits.reset();
            return notify_of_trait_change(my_traits);
        }
    }
    boost::shared_ptr< input::Traits<output::LocalizedImage> >
        rt = Engine::convert_traits(config, r->traits<engine::Image>());

    my_traits.reset( r->clone() );
    my_traits->set_traits(rt);
    DEBUG("Setting traits variable thres for traits " << my_traits.get() << " and basename " << &my_traits->suggested_output_basename);
    my_traits->suggested_output_basename.set_variable
        ( "thres", amplitude_threshold_string() );
    DEBUG("Basename is now " << my_traits->suggested_output_basename << " for my traits " << my_traits.get() );
    notify_of_trait_change(my_traits);
    DEBUG("Finished notifying, traits are " << my_traits.get() << " " << current_traits().get());
    return AtEnd();
}

ChainLink::AtEnd
ChainLink::context_changed(ContextRef r, Link* l)
{
    Link::context_changed(r, l);
    if ( r->throw_errors && current_traits().get() == NULL )
        throw std::runtime_error("rapidSTORM engine cannot process the given data");

    my_context.reset( r->clone() );
    typedef input::Traits<dStorm::engine::Image> ImageTraits;
    if ( ! my_context->has_info_for<engine::Image>() )
        my_context->more_infos.push_back( new ImageTraits() );
    assert( my_context->has_info_for<engine::Image>() );

    my_context->will_make_multiple_passes = true;
    my_context->need_multiple_concurrent_iterators = true;

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

    if ( ! config.amplitude_threshold().is_set() )
        reqs.background_stddev.require( deferred::JobTraits );
    if ( config.spotFindingMethod.isValid() )
        config.spotFindingMethod.value().set_requirements(reqs);
    if ( config.spotFittingMethod.isValid() )
        config.spotFittingMethod.value().set_requirements(reqs);
}

void ChainLink::operator()( const simparm::Event& e ) {
    ost::MutexLock lock( input::global_mutex() );
    /* TODO: if ( &e.source == &config.fixSigma.value ) {
        my_context->will_make_multiple_passes = ! config.fixSigma();
        notify_of_context_change( my_context );
    } else */ if ( &e.source == &config.spotFindingMethod.value ) {
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
    }
}

}
}
