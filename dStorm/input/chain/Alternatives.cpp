#define VERBOSE
#include "../debug.h"

#include "Alternatives.h"
#include "Context.h"
#include "Forwarder.h"
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Message.hh>
#include <map>
#include "Filter.h"
#include "../InputMutex.h"

namespace dStorm {
namespace input {
namespace chain {

class Alternatives::UpstreamCollector 
: public Forwarder, boost::noncopyable
{
    TraitsRef traits;
    ContextRef null_context;
    typedef std::map<std::string,ContextRef> Contexts;
    Contexts contexts;
    Alternatives& papa;
    bool frozen, action_in_frozen;

  public:
    UpstreamCollector(Alternatives& papa) : papa(papa), frozen(false) {}
    UpstreamCollector(Alternatives& papa, const UpstreamCollector& o)
        : null_context(o.null_context), contexts(o.contexts), papa(papa), frozen(false) {}

    UpstreamCollector* clone() const 
        { assert(false); throw std::logic_error("UpstreamCollector unclonable"); }

    BaseSource* makeSource() { return Forwarder::makeSource(); }

    AtEnd notify_of_context_change( ContextRef ctx ) {
        if ( frozen ) {
            action_in_frozen = true;
            return AtEnd();
        } else
            return Forwarder::notify_of_context_change(ctx);
    }

    AtEnd traits_changed( TraitsRef r, Link* from ) {
        DEBUG("Traits changed from upstream in " << this  << " from " << from << " to " << r.get() );
        Link::traits_changed(r,from);
        traits = r;
        AtEnd rv;
        for ( Alternatives::iterator i = papa.beginChoices(); i != papa.endChoices(); ++i )
            rv = i->traits_changed( r, this );
        return rv;
    }
    AtEnd context_changed( ContextRef r, Link* from ) {
        Link::context_changed(r,from);
        DEBUG("Context for " << from->getNode().getName() << " to " << this << " changed to " << r.get());
        contexts[from->getNode().getName()] = r;
        if ( from == &(papa.value()) ) {
            DEBUG("Forwarding context");
            return notify_of_context_change( r );
        } else {
            DEBUG("No action taken: " << r.get() << " " << papa.value.hasValue() << " " << from->current_traits().get());
            /* Choice is not the current choice - do nothing. */
            return AtEnd();
        }
    }

    AtEnd set_default_context( ContextRef ctx ) { 
        null_context = ctx; 
        if ( ! papa.isValid() ) {
            AtEnd rv = notify_of_context_change(ctx);
            return rv;
        } else {
            if ( ctx->throw_errors && ! traits.get() )
                throw std::runtime_error("The choice for " + papa.getDesc() + " cannot handle the current input");
            return AtEnd();
        }
    }

    void chose_alternative( Link* link ) {
        if ( !link )
            notify_of_context_change( null_context );
        else if ( contexts.find(link->getNode().getName() ) != contexts.end() ) {
            notify_of_context_change( contexts[link->getNode().getName()] );
        }
    }

    bool has_valid_context( Link *l ) const { 
        if ( l == NULL ) return false;
        Contexts::const_iterator probe = contexts.find(l->getNode().getName());
        DEBUG("Checking if context for " << l << " is valid: " << ( probe != contexts.end() && probe->second.get() != NULL ));
        return ( probe != contexts.end() && probe->second.get() != NULL );
    }

    void set_upstream_element( Link& element, SetType type ) {
        if ( type == Add ) {
            assert( papa.hasChoice( element.getNode().getName() ) );
        }
    }

    void freeze() { action_in_frozen = false; frozen = true; }
    void thaw() { 
        frozen = false; 
        if ( action_in_frozen )  {
            if ( papa.isValid() )
                notify_of_context_change( contexts[papa.value().getNode().getName()] );
            else
                notify_of_context_change( null_context );
        }
    }
};

Alternatives::Alternatives(std::string name, std::string desc)
: simparm::NodeChoiceEntry<Filter>(name, desc),
  simparm::Listener( simparm::Event::ValueChanged ),
  collector( new UpstreamCollector(*this) )
{
    simparm::NodeChoiceEntry<Filter>::set_auto_selection( false );
    receive_changes_from( value );
}

Alternatives::Alternatives(const Alternatives& o)
: Link(o), simparm::NodeChoiceEntry<Filter>(o, simparm::NodeChoiceEntry<Filter>::NoCopy),
  simparm::Listener( simparm::Event::ValueChanged ),
  collector( new UpstreamCollector(*this, *o.collector) )
{
    receive_changes_from( value );
}

Alternatives::~Alternatives() {
    stop_receiving_changes_from(value);
}

void Alternatives::set_more_specialized_link_element( Link* l ) {
    collector->set_more_specialized_link_element( l );
}
    
    
void Alternatives::operator()(const simparm::Event&)
{
    ost::MutexLock lock( global_mutex() );
    DEBUG("Value changed for alternatives");
    if ( value.hasValue() ) {
        DEBUG("Propagating traits for new value");
        notify_of_trait_change( value().current_traits() );
    }
    if ( value.hasValue() )
        collector->chose_alternative( &value() );
    else
        collector->chose_alternative( NULL );
}

Link::AtEnd Alternatives::traits_changed( TraitsRef t, Link* from ) {
    Link::traits_changed(t,from);
    DEBUG("Alternatives " << this << " traits changed to " << t.get() << " from " << from);
    if ( &( value() ) == from && t.get() == NULL ) {
        DEBUG("Auto-deselecting value");
        /* Choice can deliver no traits, i.e. is invalid. Find valid one. */
        bool found = false;
        for ( iterator i = beginChoices(); i != endChoices(); ++i ) {
            if ( i->current_traits().get() != NULL ) {
                value = &(*i);
                found = true;
            }
        }
        if ( ! found ) 
            value = NULL;
    } else if ( ! isValid() && t.get() != NULL )
    {
        value = dynamic_cast<Filter*>(from);
    }
    DEBUG("Finished possible auto-deselection");
    if ( from == &( value() ) ) {
        DEBUG("Notifying of trait change for traits " << t.get());
        return notify_of_trait_change( t );
        DEBUG("Notified of trait change");
    } else {
        /* Reporting choice is not current choice */
        return AtEnd();
    }
}

Link::AtEnd Alternatives::context_changed( ContextRef context, Link* link ) {
    DEBUG("Alternatives context changed");
    AtEnd rv = Link::context_changed(context, link);
    collector->freeze();
    current_context = context;
    if ( context.get() ) {
        no_throw_context.reset( context->clone() );
        no_throw_context->throw_errors = false;
    } else
        no_throw_context.reset();
    collector->set_default_context( context );
    for ( iterator i = beginChoices(); i != endChoices(); ++i ) {
        if ( &value() == &*i )
            rv = i->context_changed( current_context, this );
        else
            rv = i->context_changed( no_throw_context, this );
    }
    collector->thaw();
    if ( context->throw_errors && !isValid() )
        throw std::runtime_error("No alternative selected for '" + getDesc() + "'");
    else if ( context->throw_errors )
        DEBUG("Alternative " << value().getNode().getName() << " selected");
    DEBUG("Processed alternatives context change");
    return rv;
}

BaseSource* Alternatives::makeSource() {
    if ( ! isValid() ) throw std::runtime_error("No alternative selected for '" + getDesc() + "'");
    return value().makeSource();
}

Alternatives* Alternatives::clone() const 
    { return new Alternatives(*this); }
simparm::Node& Alternatives::getNode() {
    return *this;
}

void Alternatives::add_choice( Filter& choice, ChoiceEntry::iterator where) 
{
    this->addChoice( where, choice );
    Alternatives::set_upstream_element( choice, *this, Add );
    choice.set_more_specialized_link_element( collector.get() );
    if ( current_context.get() != NULL )
        choice.context_changed( current_context, this );
    traits_changed( choice.current_traits(), &choice );
}
void Alternatives::push_back_choice( Filter& c) {
    add_choice( c, endChoices() );
}

void Alternatives::remove_choice( Filter& choice ) {
    choice.set_more_specialized_link_element( NULL );
    Alternatives::set_upstream_element( choice, *this, Remove );
    this->removeChoice( choice );
}

void Alternatives::downstream_element_destroyed( Link& which )
{
    /* Only a static cast here since the Filter part of the object *
     * is most probably already destroyed. */
    removeChoice_noNode( static_cast<Filter&>(which) );
}

void Alternatives::throw_exception_for_invalid_configuration() const
{
    throw std::runtime_error("No alternative chosen for '" + getDesc() + "'");
}

void Alternatives::insert_new_node( std::auto_ptr<Link> link, Place p )
{
    collector->insert_new_node(link,p);
}

}
}
}
