#include "debug.h"
#include "join.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ChoiceEntry_Iterator.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Entry.hh>
#include <boost/lexical_cast.hpp>
#include <boost/mpl/for_each.hpp>
#include <vector>
#include "Source.h"
#include <dStorm/input/chain/DefaultFilterTypes.h>
#include <dStorm/input/chain/MetaInfo.h>

#include "join/spatial.hpp"
#include "join/temporal.hpp"
#include "join/iterator_generic.hpp"

namespace dStorm {
namespace input {
namespace join {

typedef std::vector< boost::shared_ptr<BaseSource> > Sources;

struct fluorophore_tag {
    static std::string get_name() { return "Fluorophore"; }
    static std::string get_desc() { return "By fluorophore index"; }
};

struct Strategist
: public simparm::Object
{
    Strategist(std::string name, std::string desc) : Object(name, desc) {}
    virtual Strategist* clone() const = 0;
    virtual boost::shared_ptr< const chain::MetaInfo > make_traits(
        const std::vector< boost::shared_ptr< const chain::MetaInfo > >& v ) = 0;
    virtual std::auto_ptr<BaseSource> make_source( const Sources& sources ) = 0;
    virtual ~Strategist() {}
};

template <typename Type, typename Tag>
class Source
: public input::Source<Type>
{
    typedef input::Source<Type> Base;
    typedef std::vector< boost::shared_ptr<Base> > Sources;
    Sources sources;
    std::vector< boost::shared_ptr< const input::Traits<Type> > > base_traits;
    typename Base::TraitsPtr traits;

  public:
    Source( const Sources& s ) : input::Source<Type>(*s[0]), sources(s) {}
    void dispatch(BaseSource::Messages m) {
        for (typename Sources::iterator i = sources.begin(); i != sources.end(); ++i)
            (*i)->dispatch(m);
    }
    typename Base::iterator begin();
    typename Base::iterator end();
    typename Base::TraitsPtr get_traits() {
        for (typename Sources::iterator i = sources.begin(); i != sources.end(); ++i) {
            base_traits.push_back( (*i)->get_traits() );
        }
        traits.reset(  merge_traits<Type,Tag>()(base_traits).release()  );
        return traits;
    }
};

template <typename Type, typename Tag>
typename input::Source<Type>::iterator
    Source<Type,Tag>::begin() 
{ 
    return typename Base::iterator( iterator<Type,Tag>(traits, base_traits, sources, false) );  
}
template <typename Type, typename Tag>
typename input::Source<Type>::iterator
Source<Type,Tag>::end()  {
    return typename Base::iterator( iterator<Type,Tag>(traits, base_traits, sources, true) );
}

template <int Dim>
std::auto_ptr< BaseSource > make_specialized_source( 
        const std::vector< boost::shared_ptr< input::Source<engine::Image> > >& v ,
        spatial_tag<Dim>
) { 
    return std::auto_ptr< BaseSource >( new Source<engine::Image, spatial_tag<Dim> >(v) ); 
}

template <typename Type>
std::auto_ptr< BaseSource > make_specialized_source( 
        const std::vector< boost::shared_ptr< input::Source<Type> > >& v ,
        temporal_tag
) { 
    std::auto_ptr< BaseSource > rv( new Source<Type, temporal_tag >(v) ); 
    return rv;
}

template <typename Type, typename Tag>
std::auto_ptr< BaseSource > make_specialized_source( 
        const std::vector< boost::shared_ptr< input::Source<Type> > >& v ,
        Tag
) { 
    throw std::runtime_error("Sorry, joining input files with these types not implemented yet.");
}

template <typename Tag>
class StrategistImplementation
: public Strategist
{
  public:
    typedef void result_type;
    StrategistImplementation() : Strategist(Tag::get_name(), Tag::get_desc()) {}
    StrategistImplementation* clone() const { return new StrategistImplementation(*this); }

    template <typename Type>
    void operator()( 
        const std::vector< boost::shared_ptr< const chain::MetaInfo > >& v,
        const Type&,
        boost::shared_ptr< const chain::MetaInfo >& result
    ) {
        if ( result.get() ) return;
        typename traits_merger<Type>::argument_type traits;
        for (size_t i = 0; i < v.size(); ++i)
            if ( v[i] && v[i]->provides< Type >() ) {
                traits.push_back( v[i]->traits<Type>() );
                assert( traits.back() );
            } else
                return;
        std::auto_ptr<BaseTraits> base( merge_traits<Type,Tag>()( traits ).release() );
        if ( ! base.get() ) return;
        std::auto_ptr< chain::MetaInfo > rv(new chain::MetaInfo(*v[0]) );
        rv->set_traits( base );
        result.reset( rv.release() );
    }

    template <typename Type>
    void operator()( 
        const Sources& sources, const Type&,
        std::auto_ptr<BaseSource>& result
    ) {
        if ( result.get() ) return;
        std::vector< boost::shared_ptr< input::Source<Type> > > typed;
        for (size_t i = 0; i < sources.size(); ++i) {
            typed.push_back( 
                boost::dynamic_pointer_cast< input::Source<Type>, BaseSource >( sources[i] ) );
            if ( ! typed.back().get() ) return;
        }
        result = make_specialized_source( typed, Tag() );
    }

    boost::shared_ptr< const chain::MetaInfo > make_traits(
        const std::vector< boost::shared_ptr< const chain::MetaInfo > >& v ) 
    {
        boost::shared_ptr<const chain::MetaInfo> result;
        boost::mpl::for_each< chain::DefaultTypes >(
            boost::bind( boost::ref(*this), boost::ref(v), _1, boost::ref(result) ) );
        return result;
    }
    std::auto_ptr<BaseSource> make_source( const Sources& sources ) { 
        std::auto_ptr<BaseSource> result;
        boost::mpl::for_each< chain::DefaultTypes >(
            boost::bind( boost::ref(*this), boost::ref(sources), _1, boost::ref(result) ) );
        return result;
    }
};

class Link 
: public chain::Link, public simparm::Object, public simparm::Listener
{
    boost::ptr_vector< simparm::Object > connection_nodes;
    boost::ptr_vector< chain::Link > children;
    ContextRef current_context;
    std::vector< TraitsRef > input_traits;
    simparm::Set channels;
    simparm::NodeChoiceEntry< Strategist > join_type;
    simparm::Entry<unsigned long> channel_count;

  protected:
    virtual void operator()(const simparm::Event&);

  public:
    Link( std::string name, std::string desc, std::auto_ptr<chain::Link> child );
    Link( const Link& );
    ~Link();
    Link* clone() const { return new Link(*this); }

    AtEnd traits_changed( TraitsRef, chain::Link* );
    AtEnd context_changed( ContextRef, chain::Link* );

    BaseSource* makeSource();
    simparm::Node& getNode() { return *this; }

    void insert_new_node( std::auto_ptr<chain::Link>, Place );
    operator const simparm::Node&() const { return *this; }
    operator simparm::Node&() { return *this; }
};

Link::Link( std::string name, std::string desc, std::auto_ptr<chain::Link> child )
: simparm::Object("MultiChannel", "Multi-channel input"),
  simparm::Listener( simparm::Event::ValueChanged ),
  channels("Channels", "Channels"),
  join_type("JoinOn", "Join inputs on"),
  channel_count("ChannelCount", "Number of input channels", 1)
{
    channel_count.min = 1;
    channel_count.helpID = "#join.ChannelCount";
    join_type.helpID = "#join.JoinOn";

    push_back( channel_count );
    push_back( channels );
    push_back( join_type );

    input_traits.push_back( child->current_traits() );
    connection_nodes.push_back( new simparm::Object("Channel1", "Channel 1") );
    connection_nodes[0].push_back( *child );
    channels.push_back(connection_nodes[0]);
    children.push_back( child );
    set_upstream_element( children.back(), *this, Add );
    join_type.addChoice( new StrategistImplementation< spatial_tag<0> >() );
    join_type.addChoice( new StrategistImplementation< spatial_tag<1> >() );
    join_type.addChoice( new StrategistImplementation< spatial_tag<2> >() );
    join_type.addChoice( new StrategistImplementation< temporal_tag >() );
    // TODO: Implement join_type.addChoice( new StrategistImplementation< fluorophore_tag >() );
    join_type.choose( spatial_tag<2>::get_name() );
    join_type.viewable = false;
    channels.showTabbed = true;

    receive_changes_from( join_type.value );
    receive_changes_from( channel_count.value );
}

Link::Link( const Link& o )
: chain::Link(o), simparm::Object(o), 
  simparm::Listener( simparm::Event::ValueChanged ),
  connection_nodes(o.connection_nodes),
  children( o.children ), current_context( o.current_context ),
  input_traits( o.input_traits ),
  channels(o.channels), join_type( o.join_type ), channel_count(o.channel_count)
{
    assert( children.size() == connection_nodes.size() );
    for (size_t i = 0; i < children.size() ; ++i) {
        connection_nodes[i].push_back(children[i]);
        channels.push_back(connection_nodes[i]);
        set_upstream_element( children[i], *this, Add );
    }
    receive_changes_from( join_type.value );
    receive_changes_from( channel_count.value );

    push_back( channel_count );
    push_back( channels );
    push_back( join_type );
}

Link::~Link() {
}

Link::AtEnd Link::traits_changed( TraitsRef r, chain::Link* l ) {
    assert( children.size() == input_traits.size() );
    for (size_t i = 0; i < children.size(); ++i)
        if ( &children[i] == l )
            input_traits[i] = r;
    TraitsRef t;
    if ( children.size() == 1 ) {
        t = input_traits[0];
    } else {
        try {
            t = join_type().make_traits( input_traits ) ;
        } catch (const std::runtime_error&) {}
    }
    return notify_of_trait_change(t);
}

Link::AtEnd Link::context_changed( ContextRef c, chain::Link* l ) {
    current_context = c;
    for (size_t i = 0; i < children.size(); ++i)
        children[i].context_changed( c, this );
    return AtEnd();
}

BaseSource* Link::makeSource() {
    if ( children.size() == 1 ) {
        return children[0].makeSource();
    } else {
        Sources sources;
        for (size_t i = 0; i < children.size(); ++i)
            sources.push_back( boost::shared_ptr<BaseSource>( 
                children[i].makeSource() ) );
        return join_type().make_source( sources ).release();
    }
}

void Link::insert_new_node( std::auto_ptr<chain::Link> l, Place p ) {
    for (size_t i = 1; i < children.size(); ++i)
        children[i].insert_new_node( std::auto_ptr<chain::Link>( l->clone() ), p );
    children[0].insert_new_node(l,p);
}

void Link::operator()(const simparm::Event& e) {
    if ( &e.source == &join_type.value ) {
        TraitsRef t;
        try {
            t = join_type().make_traits( input_traits ) ;
        } catch (const std::runtime_error&) {}
        notify_of_trait_change(t);
    } else if ( &e.source == &channel_count.value ) {
        DEBUG("Channel count changed to " << channel_count());
        join_type.viewable = channel_count() > 1;
        while ( children.size() < channel_count() ) {
            children.push_back( children[0].clone() );
            std::string i = boost::lexical_cast<std::string>(children.size());
            connection_nodes.push_back( new simparm::Object("Channel" + i, "Channel " + i) );
            connection_nodes.back().push_back( children.back() );
            channels.push_back( connection_nodes.back() );
            input_traits.push_back( children.back().current_traits() );
            set_upstream_element( children.back(), *this, Add );
        }
        while ( children.size() > channel_count() ) {
            children.pop_back();
            connection_nodes.pop_back();
            input_traits.pop_back();
        }
        TraitsRef t;
        try {
            t = join_type().make_traits( input_traits ) ;
        } catch (const std::runtime_error& e) {}
        notify_of_trait_change(t);
    }
}

std::auto_ptr<chain::Link> create_link( std::auto_ptr<chain::Link> child )
{
    return std::auto_ptr<chain::Link>( new Link("MultiChannel", "Multi-channel input", child) );
}

}
}
}
