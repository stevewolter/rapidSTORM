#include "debug.h"
#include "inputs/join.h"
#include "simparm/ChoiceEntry.h"
#include "simparm/ManagedChoiceEntry.h"
#include "simparm/Entry.h"
#include "simparm/TabGroup.h"
#include "simparm/ObjectChoice.h"
#include <boost/lexical_cast.hpp>
#include <boost/mpl/for_each.hpp>
#include <vector>
#include "input/Source.h"
#include "input/DefaultFilterTypes.h"
#include "input/MetaInfo.h"

#include "inputs/join/spatial.h"
#include "inputs/join/temporal.h"

namespace dStorm {
using namespace input;
namespace inputs {
namespace join {

typedef std::vector<std::unique_ptr<BaseSource>> Sources;

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
    virtual std::unique_ptr<BaseSource> make_source( Sources sources ) = 0;
    void attach_ui( simparm::NodeHandle to ) { attach_parent(to); }
};

std::unique_ptr< BaseSource > make_specialized_source( 
        std::vector<std::unique_ptr<input::Source<engine::ImageStack>>> v,
        spatial_join::tag
) { 
    return std::unique_ptr< BaseSource >(spatial_join::Create(std::move(v)).release()); 
}

template <typename Type>
std::unique_ptr< BaseSource > make_specialized_source( 
        std::vector<std::unique_ptr<input::Source<Type>>> v,
        temporal_join::tag
) { 
    std::unique_ptr< BaseSource > rv(temporal_join::Create(std::move(v)).release()); 
    return rv;
}

template <typename Type, typename Tag>
std::unique_ptr< BaseSource > make_specialized_source( 
        std::vector<std::unique_ptr<input::Source<Type>>> v,
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
        std::vector< boost::shared_ptr< const input::Traits<Type> > > traits;
        for (size_t i = 0; i < v.size(); ++i)
            if ( v[i] && v[i]->provides< Type >() ) {
                traits.push_back( v[i]->traits<Type>() );
                assert( traits.back() );
            } else
                return;
        std::unique_ptr<BaseTraits> base( merge_traits( traits, Tag() ).release() );
        if ( ! base.get() ) return;
        std::unique_ptr< MetaInfo > rv(new MetaInfo(*v[0]) );
        rv->set_traits( base.release() );
        result.reset( rv.release() );
    }

    template <typename Type>
    void operator()( 
        Sources* sources, const Type&,
        std::unique_ptr<BaseSource>& result
    ) {
        if ( result.get() ) return;
        if ( sources->size() == 0 ) return;
        if (dynamic_cast<input::Source<Type>*>(sources->at(0).get()) == nullptr) {
            return;
        }

        std::vector<std::unique_ptr<input::Source<Type>>> typed;
        for (size_t i = 0; i < sources->size(); ++i) {
            typed.emplace_back(
                dynamic_cast<input::Source<Type>*>(sources->at(i).release()));
            assert(typed.back().get());
        }
        result = make_specialized_source( std::move(typed), Tag() );
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
    std::unique_ptr<BaseSource> make_source( Sources sources ) { 
        std::unique_ptr<BaseSource> result;
        boost::mpl::for_each< DefaultTypes >(
            boost::bind( boost::ref(*this), &sources, _1, boost::ref(result) ) );
        return result;
    }
};

class Link 
: public input::Link
{
    std::vector<simparm::Object> connection_nodes;
    std::vector< std::unique_ptr<input::Link> > children;
    std::vector< Connection > connections;
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
    Link* clone() const OVERRIDE { return new Link(*this); }

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

    BaseSource* makeSource() OVERRIDE;
    std::string name() const OVERRIDE { return name_object.getName(); }
    std::string description() const { return name_object.getDesc(); }
    void registerNamedEntries( simparm::NodeHandle n ) OVERRIDE { 
        listening[0] = join_type.value.notify_on_value_change( 
            boost::bind( &Link::recompute_meta_info, this ) );
        listening[1] = channel_count.value.notify_on_value_change( 
            boost::bind( &Link::change_channel_count, this ) );

        simparm::NodeHandle r = name_object.attach_ui( n );
        channel_count.attach_ui( r );
        channels_node = channels.attach_ui( r );
        join_type.attach_ui( r );

        for (unsigned i = 0; i < children.size(); ++i) {
            children[i]->registerNamedEntries(
                    connection_nodes[i].attach_ui(channels_node) );
        }
        registered_node = true;
    }
    void publish_meta_info() OVERRIDE {
        for (unsigned i = 0; i < children.size(); ++i)
            children[i]->publish_meta_info();
        assert( current_meta_info().get() );
    }

    void insert_new_node( std::unique_ptr<input::Link> ) OVERRIDE;
};

Link::Link()
: name_object("MultiChannel", "Multi-channel input"),
  channels("Channels", "Channels"),
  join_type("JoinOn"),
  channel_count("ChannelCount", 1),
  registered_node(false)
{
    channel_count.min = 1;

    join_type.addChoice( new StrategistImplementation< spatial_join::tag >() );
    join_type.addChoice( new StrategistImplementation< temporal_join::tag >() );
    join_type.choose( spatial_join::tag::get_name() );
    join_type.set_visibility( false );
}

Link::Link( const Link& o )
: input::Link(o), 
  connection_nodes(o.connection_nodes),
  input_traits( o.input_traits ),
  name_object( o.name_object ),
  channels(o.channels), join_type( o.join_type ), channel_count(o.channel_count),
  registered_node(false)
{
    for (const auto& child : o.children) {
        children.emplace_back(child->clone());
    }

    assert( children.size() == connection_nodes.size() );
    for (auto& child : children) {
        connections.push_back( child->notify(
            boost::bind( &Link::traits_changed, this, _1, child.get() ) ) );
    }
}

Link::~Link() {
}

void Link::traits_changed( TraitsRef r, input::Link* l ) {
    assert( children.size() == input_traits.size() );
    for (size_t i = 0; i < children.size(); ++i)
        if ( children[i].get() == l )
            input_traits[i] = r;
    recompute_meta_info();
}

BaseSource* Link::makeSource() {
    if ( children.size() == 1 ) {
        return children[0]->make_source().release();
    } else {
        Sources sources;
        for (auto& child : children) {
            sources.emplace_back(child->make_source());
        }
        return join_type().make_source( std::move(sources) ).release();
    }
}

void Link::insert_new_node( std::unique_ptr<input::Link> l ) {
    if ( children.size() == 0 ) {
        input_traits.push_back( l->current_meta_info() );
        connection_nodes.emplace_back("Channel1", "Channel 1");
        children.push_back( std::move(l) );
        connections.push_back( children.back()->notify(
            boost::bind( &Link::traits_changed, this, _1, children.back().get() ) ) );
    } else {
        for (size_t i = 1; i < children.size(); ++i)
            children[i]->insert_new_node( std::unique_ptr<input::Link>( l->clone() ) );
        children[0]->insert_new_node(std::move(l));
    }
}

void Link::change_channel_count() {
    DEBUG("Channel count changed to " << channel_count());
    join_type.set_visibility( channel_count() > 1 );
    while ( children.size() < channel_count() ) {
        children.emplace_back( children[0]->clone() );
        children.back()->publish_meta_info();
        std::string i = boost::lexical_cast<std::string>(children.size());
        connection_nodes.emplace_back("Channel" + i, "Channel " + i);
        input_traits.push_back( children.back()->current_meta_info() );
        if ( registered_node && channels_node ) {
            children.back()->registerNamedEntries( 
                connection_nodes.back().attach_ui( channels_node ) );
        }
        connections.push_back( children.back()->notify(
            boost::bind( &Link::traits_changed, this, _1, children.back().get() ) ) );
    }
    while ( children.size() > channel_count() ) {
        children.pop_back();
        connection_nodes.pop_back();
        input_traits.pop_back();
    }
    recompute_meta_info();
}

std::unique_ptr<input::Link> create_link() {
    return std::unique_ptr<input::Link>( new Link() );
}

}
}
}
