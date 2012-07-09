#include "debug.h"
#include "join.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <simparm/ChoiceEntry.h>
#include <simparm/ManagedChoiceEntry.h>
#include <simparm/Entry.h>
#include <simparm/TabGroup.h>
#include <simparm/ObjectChoice.h>
#include <boost/lexical_cast.hpp>
#include <boost/mpl/for_each.hpp>
#include <vector>
#include <dStorm/input/Source.h>
#include <dStorm/input/DefaultFilterTypes.h>
#include <dStorm/input/MetaInfo.h>
#include <dStorm/make_clone_allocator.hpp>

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
: public simparm::ObjectChoice
{
    Strategist(std::string name, std::string desc) : ObjectChoice(name, desc) {}
    virtual Strategist* clone() const = 0;
    virtual boost::shared_ptr< const MetaInfo > make_traits(
        const std::vector< boost::shared_ptr< const MetaInfo > >& v ) = 0;
    virtual std::auto_ptr<BaseSource> make_source( const Sources& sources ) = 0;
    void attach_ui( simparm::NodeHandle to ) { attach_parent(to); }
};

}
}
}

DSTORM_MAKE_BOOST_CLONE_ALLOCATOR( dStorm::input::join::Strategist );

namespace dStorm {
namespace input {
namespace join {


template <typename Type, typename Tag>
class Source
: public input::Source<Type>
{
    typedef input::Source<Type> Base;
    typedef std::vector< boost::shared_ptr<Base> > Sources;
    Sources sources;
    std::vector< boost::shared_ptr< const input::Traits<Type> > > base_traits;
    typename Base::TraitsPtr traits;
    boost::ptr_vector< simparm::Object > connection_nodes;

    void attach_ui_( simparm::NodeHandle n ) { 
        for (size_t i = 0; i < sources.size(); ++i) {
            std::auto_ptr< simparm::Object > object( 
                new simparm::Object("Channel" + boost::lexical_cast<std::string>(i), "") );
            sources[i]->attach_ui( object->attach_ui( n ) ); 
            connection_nodes.push_back( object );
        }
    }

  public:
    Source( const Sources& s ) : sources(s) {}
    void dispatch(BaseSource::Messages m) {
        for (typename Sources::iterator i = sources.begin(); i != sources.end(); ++i)
            (*i)->dispatch(m);
    }
    typename Base::iterator begin();
    typename Base::iterator end();
    typename Base::TraitsPtr get_traits( input::BaseSource::Wishes r ) {
        for (typename Sources::iterator i = sources.begin(); i != sources.end(); ++i) {
            base_traits.push_back( (*i)->get_traits(r) );
        }
        traits.reset(  merge_traits<Type,Tag>()(base_traits).release()  );
        return traits;
    }

    BaseSource::Capabilities capabilities() const {
        BaseSource::Capabilities rv;
        rv.set();
        for (typename Sources::const_iterator i = sources.begin(); i != sources.end(); ++i)
            rv = rv.to_ulong() & (*i)->capabilities().to_ulong();
        return rv;
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

std::auto_ptr< BaseSource > make_specialized_source( 
        const std::vector< boost::shared_ptr< input::Source<engine::ImageStack> > >& v ,
        spatial_tag<2>
) { 
    return std::auto_ptr< BaseSource >( new Source<engine::ImageStack, spatial_tag<2> >(v) ); 
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
        const std::vector< boost::shared_ptr< const MetaInfo > >& v,
        const Type&,
        boost::shared_ptr< const MetaInfo >& result
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
        std::auto_ptr< MetaInfo > rv(new MetaInfo(*v[0]) );
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

    boost::shared_ptr< const MetaInfo > make_traits(
        const std::vector< boost::shared_ptr< const MetaInfo > >& v ) 
    {
        boost::shared_ptr<const MetaInfo> result;
        boost::mpl::for_each< DefaultTypes >(
            boost::bind( boost::ref(*this), boost::ref(v), _1, boost::ref(result) ) );
        if ( ! result ) result.reset( new MetaInfo() );
        return result;
    }
    std::auto_ptr<BaseSource> make_source( const Sources& sources ) { 
        std::auto_ptr<BaseSource> result;
        boost::mpl::for_each< DefaultTypes >(
            boost::bind( boost::ref(*this), boost::ref(sources), _1, boost::ref(result) ) );
        return result;
    }
};

class Link 
: public input::Link
{
    boost::ptr_vector< simparm::Object > connection_nodes;
    boost::ptr_vector< input::Link > children;
    boost::ptr_vector< boost::signals2::scoped_connection > connections;
    std::vector< TraitsRef > input_traits;
    simparm::Object name_object;
    simparm::TabGroup channels;
    simparm::ManagedChoiceEntry< Strategist > join_type;
    simparm::Entry<unsigned long> channel_count;
    bool registered_node;

    simparm::NodeHandle channels_node;
    simparm::BaseAttribute::ConnectionStore listening[2];

    void change_channel_count();

  public:
    Link();
    Link( const Link& );
    ~Link();

  private:
    Link* clone() const { return new Link(*this); }

    void traits_changed( TraitsRef, input::Link* );
    void recompute_meta_info() {
        TraitsRef t;
        if ( children.size() == 1 ) {
            t = input_traits[0];
        } else {
            DEBUG("Making traits for size " << children.size());
            try {
                assert( join_type.isValid() );
                t = join_type().make_traits( input_traits ) ;
                assert( t.get() );
            } catch (const std::runtime_error&) {
                t.reset( new MetaInfo() );
            }
            DEBUG("Made traits providing nothing: " << ((t.get()) ? t->provides_nothing() : true));
        }
        update_current_meta_info(t);
    }

    BaseSource* makeSource();
    std::string name() const { return name_object.getName(); }
    std::string description() const { return name_object.getDesc(); }
    void registerNamedEntries( simparm::NodeHandle n ) { 
        listening[0] = join_type.value.notify_on_value_change( 
            boost::bind( &Link::recompute_meta_info, this ) );
        listening[1] = channel_count.value.notify_on_value_change( 
            boost::bind( &Link::change_channel_count, this ) );

        simparm::NodeHandle r = name_object.attach_ui( n );
        channel_count.attach_ui( r );
        channels_node = channels.attach_ui( r );
        join_type.attach_ui( r );

        for (unsigned i = 0; i < children.size(); ++i) {
            children[i].registerNamedEntries( connection_nodes[i].attach_ui(channels_node) );
        }
        registered_node = true;
    }
    void publish_meta_info() {
        for (unsigned i = 0; i < children.size(); ++i)
            children[i].publish_meta_info();
        assert( current_meta_info().get() );
    }

    void insert_new_node( std::auto_ptr<input::Link>, Place );
};

Link::Link()
: name_object("MultiChannel", "Multi-channel input"),
  channels("Channels", "Channels"),
  join_type("JoinOn"),
  channel_count("ChannelCount", 1),
  registered_node(false)
{
    channel_count.min = 1;

    join_type.addChoice( new StrategistImplementation< spatial_tag<2> >() );
    join_type.addChoice( new StrategistImplementation< temporal_tag >() );
    // TODO: Implement join_type.addChoice( new StrategistImplementation< fluorophore_tag >() );
    join_type.choose( spatial_tag<2>::get_name() );
    join_type.set_visibility( false );
}

Link::Link( const Link& o )
: input::Link(o), 
  connection_nodes(o.connection_nodes),
  children( o.children ), 
  input_traits( o.input_traits ),
  name_object( o.name_object ),
  channels(o.channels), join_type( o.join_type ), channel_count(o.channel_count),
  registered_node(false)
{
    assert( children.size() == connection_nodes.size() );
    for (size_t i = 0; i < children.size() ; ++i) {
        connections.push_back( children[i].notify(
            boost::bind( &Link::traits_changed, this, _1, &children[i] ) ) );
    }
}

Link::~Link() {
}

void Link::traits_changed( TraitsRef r, input::Link* l ) {
    assert( children.size() == input_traits.size() );
    for (size_t i = 0; i < children.size(); ++i)
        if ( &children[i] == l )
            input_traits[i] = r;
    recompute_meta_info();
}

BaseSource* Link::makeSource() {
    if ( children.size() == 1 ) {
        return children[0].make_source().release();
    } else {
        Sources sources;
        for (size_t i = 0; i < children.size(); ++i)
            sources.push_back( boost::shared_ptr<BaseSource>( 
                children[i].make_source() ) );
        return join_type().make_source( sources ).release();
    }
}

void Link::insert_new_node( std::auto_ptr<input::Link> l, Place p ) {
    if ( children.size() == 0 ) {
        input_traits.push_back( l->current_meta_info() );
        connection_nodes.push_back( new simparm::Object("Channel1", "Channel 1") );
        children.push_back( l );
        connections.push_back( children.back().notify(
            boost::bind( &Link::traits_changed, this, _1, &children.back() ) ) );
    } else {
        for (size_t i = 1; i < children.size(); ++i)
            children[i].insert_new_node( std::auto_ptr<input::Link>( l->clone() ), p );
        children[0].insert_new_node(l,p);
    }
}

void Link::change_channel_count() {
    DEBUG("Channel count changed to " << channel_count());
    join_type.set_visibility( channel_count() > 1 );
    while ( children.size() < channel_count() ) {
        children.push_back( children[0].clone() );
        children.back().publish_meta_info();
        std::string i = boost::lexical_cast<std::string>(children.size());
        connection_nodes.push_back( new simparm::Object("Channel" + i, "Channel " + i) );
        input_traits.push_back( children.back().current_meta_info() );
        if ( registered_node && channels_node ) {
            children.back().registerNamedEntries( 
                connection_nodes.back().attach_ui( channels_node ) );
        }
        connections.push_back( children.back().notify(
            boost::bind( &Link::traits_changed, this, _1, &children.back() ) ) );
    }
    while ( children.size() > channel_count() ) {
        children.pop_back();
        connection_nodes.pop_back();
        input_traits.pop_back();
    }
    recompute_meta_info();
}

std::auto_ptr<input::Link> create_link()
{
    return std::auto_ptr<input::Link>( new Link() );
}

}
}
}
