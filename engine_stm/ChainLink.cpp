#include "ChainLink.h"
#include "LocalizationBuncher.h"
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/output/LocalizedImage_traits.h>

namespace dStorm {
namespace engine_stm {

input::Source<output::LocalizedImage>*
ChainLink::makeSource( std::auto_ptr< input::Source<Localization> > input )
{
    return new Source<Localization>( config, input );
}

input::Source<output::LocalizedImage>*
ChainLink::makeSource( std::auto_ptr< input::Source<LocalizationFile::Record> > input )
{
    return new Source<LocalizationFile::Record>( config, input );
}

template <typename Type>
ChainLink::AtEnd
ChainLink::_traits_changed(
    TraitsRef r, Link* l,
    boost::shared_ptr< input::Traits<Type> > t)
{
    Link::traits_changed(r, l);
    input::chain::MetaInfo::Ptr rv( r->clone() );
    rv->traits.reset( new input::Traits<output::LocalizedImage>(*t) );
    return notify_of_trait_change(rv);
}

ChainLink::AtEnd
ChainLink::context_changed(ContextRef r, Link* l)
{
    Link::context_changed(r, l);
    my_context.reset( r->clone() );
    my_context->need_multiple_concurrent_iterators = false;
    return notify_of_context_change( my_context );
}

std::auto_ptr<input::chain::Filter>
make_STM_engine_link()
{
    return std::auto_ptr<input::chain::Filter>( new ChainLink( ) );
}

}
}
