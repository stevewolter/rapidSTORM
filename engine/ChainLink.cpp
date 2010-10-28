#include "ChainLink.h"
#include "Engine.h"
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/output/LocalizedImage_traits.h>

namespace dStorm {
namespace engine {

ChainLink::ChainLink() 
: simparm::Listener( simparm::Event::ValueChanged )
{
    receive_changes_from( config.fixSigma );
}

ChainLink::ChainLink(const ChainLink& c)
: input::chain::TypedFilter<engine::Image>(c),
  ClassicEngine(c),
  simparm::Listener( simparm::Event::ValueChanged ),
  my_context(c.my_context), config(c.config)
{
    receive_changes_from( config.fixSigma );
}

input::Source<output::LocalizedImage>*
ChainLink::makeSource( std::auto_ptr< input::Source<dStorm::engine::Image> > input )
{
    return new Engine( config, input );
}

ChainLink::AtEnd
ChainLink::traits_changed(TraitsRef r, Link* l, ObjectTraitsPtr t)
{
    Link::traits_changed(r, l);
    boost::shared_ptr< input::Traits<output::LocalizedImage> >
        rt = Engine::convert_traits(config, t);
    input::chain::MetaInfo::Ptr rv( r->clone() );
    rv->traits = rt;
    return notify_of_trait_change(rv);
}

ChainLink::AtEnd
ChainLink::context_changed(ContextRef r, Link* l)
{
    Link::context_changed(r, l);
    my_context.reset( r->clone() );
    my_context->will_make_multiple_passes = ! config.fixSigma();
    return notify_of_context_change( my_context );
}

std::auto_ptr<input::chain::Filter>
make_rapidSTORM_engine_link()
{
    return std::auto_ptr<input::chain::Filter>( new ChainLink( ) );
}

void ChainLink::operator()( const simparm::Event& ) {
    my_context->will_make_multiple_passes = ! config.fixSigma();
    notify_of_context_change( my_context );
}

}
}
