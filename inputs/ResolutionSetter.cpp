#include "debug.h"
#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/Eigen.hh>
#include "ResolutionSetter.h"
#include <dStorm/input/Source_impl.h>
#include <dStorm/input/chain/Context_impl.h>
#include <dStorm/engine/Image.h>
#include <dStorm/Localization.h>
#include <dStorm/input/LocalizationTraits.h>
#include <dStorm/input/InputMutex.h>
#include <dStorm/input/chain/Filter_impl.h>
#include <dStorm/input/chain/DefaultFilterTypes.h>
#include <dStorm/ImageTraits_impl.h>
#include <boost/lexical_cast.hpp>
#include <boost/lexical_cast.hpp>
#include <simparm/ChoiceEntry_Impl.hh>
#include <dStorm/input/ResolutionChange.h>

namespace dStorm {
namespace input {

namespace chain {

template <>
template <typename Type>
bool DefaultVisitor<resolution::SourceConfig>::operator()( Traits<Type>& traits )
{
    config.set_traits( traits );
    return true;
}

template <>
template <typename Type>
bool DefaultVisitor<resolution::SourceConfig>::operator()( std::auto_ptr< Source<Type> > s )
{
    assert( s.get() );
    new_source.reset( new resolution::Source<Type>(s, config) );
    return true;
}

}

namespace resolution {

using namespace chain;

template <typename ForwardedType>
typename Source<ForwardedType>::TraitsPtr
Source<ForwardedType>::get_traits()
{
    DEBUG("Setting traits in ResolutionSetter");
    TraitsPtr rv = s->get_traits();
    config.set_traits(*rv);
    DEBUG(this << " set traits in ResolutionSetter");
    return rv;
}

ChainLink::ChainLink() 
{
    DEBUG("Making ResolutionSetter chain link");
    receive_changes_from_subtree( config );
}

ChainLink::ChainLink(const ChainLink& o) 
: Filter(o),
  config(o.config), context(o.context)
{
    receive_changes_from_subtree( config );
}

ChainLink::AtEnd ChainLink::traits_changed( TraitsRef c, Link* l ) { 
    if ( c.get() )
        c->get_signal< ResolutionChange >()( config.get_resolution() );
    if ( c.get() && c->provides< dStorm::engine::Image >() ) {
        config.read_traits( *c->traits< dStorm::engine::Image >() );
    }
    return input::chain::DelegateToVisitor::traits_changed(*this, c, l);
}

chain::Link::AtEnd ChainLink::context_changed( ContextRef c, Link *l )
{
    this->context = c;
    return input::chain::DelegateToVisitor::context_changed(*this, c, l);
}

void ChainLink::operator()(const simparm::Event& e)
{
    if ( e.cause == simparm::Event::ValueChanged) {
	ost::MutexLock lock( global_mutex() );
        DefaultVisitor<SourceConfig> m(config);
        if ( context.get() ) {
            visit_context( m, context );
            notify_of_context_change( context );
        }
        if ( current_traits().get() ) {
            MetaInfo::ConstPtr t( upstream_traits() );
            visit_traits( m, t );
            notify_of_trait_change( t );
            t->get_signal< ResolutionChange >()( config.get_resolution() );
        }
    } else 
	TreeListener::add_new_children(e);
}

BaseSource* ChainLink::makeSource() { 
    DefaultVisitor<SourceConfig> visitor(config);
    return specialize_source( visitor, Forwarder::makeSource() );
}

std::auto_ptr<chain::Forwarder> makeLink() {
    DEBUG("Making resolution chain link");
    return std::auto_ptr<chain::Forwarder>( new ChainLink() );
}

}
}
}

