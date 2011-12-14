#ifndef DSTORM_INPUT_CHAIN_FILTER_IMPL_H
#define DSTORM_INPUT_CHAIN_FILTER_IMPL_H

#include "../../debug.h"
#include "Filter.h"

#include "Context_impl.h"
#include "MetaInfo.h"

#include <boost/mpl/for_each.hpp>

namespace dStorm {
namespace input {
namespace chain {

template <typename TryType, typename VisitorType>
bool apply_visitor( VisitorType& visitor, const BaseTraits& traits )
{
    const Traits<TryType>* t = dynamic_cast< const input::Traits<TryType>* >(&traits);
    if ( t != NULL )
        return visitor( *t );
    else
        return false;
}

template <typename TryType, typename VisitorType>
bool apply_visitor( VisitorType& visitor, BaseTraits& traits )
{
    Traits<TryType>* t = dynamic_cast< input::Traits<TryType>* >(&traits);
    if ( t != NULL )
        return visitor( *t );
    else
        return false;
}

template <typename TryType, typename VisitorType>
bool apply_visitor( VisitorType& visitor, std::auto_ptr<BaseSource>& source )
{
    if ( source.get() == NULL ) return false;
    Source<TryType>* t = dynamic_cast< Source<TryType>* >( source.get() );
    if ( t != NULL ) {
        std::auto_ptr< Source<TryType> > p( t );
        source.release();
        bool rv = visitor( p );
        if ( ! rv ) source.reset( p.release() );
        assert( rv || source.get() );
        return rv;
    } else
        return false;
}

template <typename Config>
struct DefaultVisitor {
    typedef typename Config::SupportedTypes SupportedTypes;

    const Config& config;
    DefaultVisitor( const Config& config ) : config(config) {}

    boost::shared_ptr<BaseTraits> new_traits;
    std::auto_ptr<BaseSource> new_source;

    template <typename Type>
    inline bool operator()( const input::Traits<Type>& traits ) {
        boost::shared_ptr< input::Traits<Type> > rv( traits.clone() );
        new_traits = rv;
        return (*this)( *rv );
    }
    template <typename Type>
    inline bool operator()( input::Traits<Type>& traits );
    template <typename Type>
    inline bool operator()( std::auto_ptr< input::Source<Type> > p );

    inline bool unknown_trait(std::string trait_desc) const;
    inline bool no_context_visited_is_ok() const;
    inline void unknown_base_source() const;

    inline bool operator()( MetaInfo& );
    inline bool operator()( Context& );
};

template <typename Config>
bool DefaultVisitor<Config>::operator()( MetaInfo& ) { return true; }

template <typename Config>
bool DefaultVisitor<Config>::operator()( Context& ) { return true; }

template <typename Config>
bool DefaultVisitor<Config>::unknown_trait(std::string) const { return true; }

template <typename Config>
bool DefaultVisitor<Config>::no_context_visited_is_ok() const { return true; }

template <typename Config>
void DefaultVisitor<Config>::unknown_base_source() const {}

struct IgnoreErrors { void operator()() const {} };

template <typename Visitor, typename Argument>
struct apply_until_match_wrapper {
    Visitor& v;
    Argument& a;
    bool& result;
    apply_until_match_wrapper(Visitor& v, Argument& a, bool& result) 
        :v(v), a(a), result(result) {}

    typedef void result_type;
    template <typename Try>
    void operator()( Try t ) 
    {
        if ( !result ) {
            result = apply_visitor< Try, Visitor >(v, a);
        }
        
    }
};

template <typename Visitor, typename Argument>
apply_until_match_wrapper<Visitor,Argument>
apply_until_match(Visitor& v, Argument& a, bool& result) { 
    return apply_until_match_wrapper<Visitor,Argument>(v,a,result); 
}

template <typename Visitor>
void visit_traits
    ( Visitor &visitor, MetaInfo::ConstPtr& t )
{
    if ( t.get() != NULL && ! t->provides_nothing() ) {
        bool one_matched = false;
        boost::mpl::for_each<typename Visitor::SupportedTypes>(
            apply_until_match( visitor, t->base_traits(), one_matched ) );
        if ( one_matched ) {
            MetaInfo::Ptr nt( t->clone() );
            nt->set_traits( visitor.new_traits );
            if ( visitor( *nt ) );
                t = nt;
        } else if ( ! visitor.unknown_trait(t->base_traits().desc()) )
            t.reset();
    }
}

template <typename Visitor>
void visit_context
    ( Visitor &visitor, Context::ConstPtr& c )
{
    if ( c.get() ) {
        input::chain::Context::Ptr p( c->clone() );
        bool some_context_visited = false;
        for ( boost::ptr_vector<BaseTraits>::iterator i = p->more_infos.begin(); i != p->more_infos.end(); ++i ) 
        {
            boost::mpl::for_each<typename Visitor::SupportedTypes>(
                apply_until_match(visitor, *i, some_context_visited) );
        }
        if ( some_context_visited || visitor.no_context_visited_is_ok() ) {
            DEBUG("Was able to apply to context or applying to unvisited context");
            if ( visitor( *p ) ) {
                DEBUG("Context successfully applied");
                c = p;
            }
        } else {
            c.reset();
        }
    }
}

template <typename Visitor>
BaseSource* specialize_source(Visitor &visitor, BaseSource* src) {
    DEBUG("Specializing source");
    std::auto_ptr<BaseSource> b(src);
    assert( b.get() );
    bool some_source_created = false;
    boost::mpl::for_each<typename Visitor::SupportedTypes>(
        apply_until_match(visitor, b, some_source_created) );
    assert( some_source_created || b.get() );
    if ( some_source_created ) {
        assert( visitor.new_source.get() );
        DEBUG("Made source");
        return visitor.new_source.release();
    } else {
        assert( b.get() );
        DEBUG("Unable to make source");
        visitor.unknown_base_source();
        return b.release();
    }
}

struct DelegateToVisitor {

    template <typename Type>
    static Link::AtEnd traits_changed( Type& f, Link::TraitsRef r, Link* l) 
    {
        f.Link::traits_changed(r,l);
        typename Type::Visitor visitor(f.get_config());
        visit_traits( visitor, r );
        return f.notify_of_trait_change(r);
    }
    template <typename Type>
    static Link::AtEnd context_changed( Type& f, Link::ContextRef r, Link* l) 
    {
        f.Link::context_changed(r,l);
        typename Type::Visitor visitor(f.get_config());
        visit_context( visitor, r );
        return f.notify_of_context_change(r);
    }
    template <typename Type>
    static BaseSource* makeSource( Type& f) 
    {
        DEBUG("Making source");
        typename Type::Visitor visitor(f.get_config());
        BaseSource* base = f.Forwarder::makeSource();
        DEBUG("Specializing base source");
        return specialize_source(visitor, base);
    }

};

}
}
}


#endif
