#include "inputs/join.h"

#include <vector>

#include <boost/lexical_cast.hpp>

#include "debug.h"
#include "helpers/make_unique.hpp"
#include "engine/InputTraits.h"
#include "input/Link.hpp"
#include "input/MetaInfo.h"
#include "inputs/join/spatial.h"
#include "inputs/join/temporal.h"
#include "input/Source.h"
#include "output/LocalizedImage_traits.h"
#include "simparm/ChoiceEntry.h"
#include "simparm/Entry.h"
#include "simparm/ManagedChoiceEntry.h"
#include "simparm/ObjectChoice.h"
#include "simparm/TabGroup.h"

namespace dStorm {
using namespace input;
namespace inputs {
namespace join {

struct fluorophore_tag {
    static std::string get_name() { return "Fluorophore"; }
    static std::string get_desc() { return "By fluorophore index"; }
};

template <typename Type>
struct Strategist
: public simparm::ObjectChoice
{
    typedef std::vector<std::unique_ptr<Source<Type>>> Sources;

    Strategist(std::string name, std::string desc) : ObjectChoice(name, desc) {}
    virtual Strategist* clone() const = 0;
    virtual boost::shared_ptr< const MetaInfo > make_traits(
        const std::vector< boost::shared_ptr< const MetaInfo > >& v ) = 0;
    virtual std::unique_ptr<Source<Type>> make_source( Sources sources ) = 0;
    void attach_ui( simparm::NodeHandle to ) { attach_parent(to); }
};

std::unique_ptr< Source<engine::ImageStack> > make_specialized_source( 
        std::vector<std::unique_ptr<input::Source<engine::ImageStack>>> v,
        spatial_join::tag
) { 
    return std::unique_ptr< Source<engine::ImageStack> >(spatial_join::Create(std::move(v)).release()); 
}

template <typename Type>
std::unique_ptr< input::Source<Type> > make_specialized_source( 
        std::vector<std::unique_ptr<input::Source<Type>>> v,
        temporal_join::tag
) { 
    std::unique_ptr< input::Source<Type> > rv(temporal_join::Create(std::move(v)).release()); 
    return rv;
}

template <typename Type, typename Tag>
std::unique_ptr< Source<Type> > make_specialized_source( 
        std::vector<std::unique_ptr<input::Source<Type>>> v,
        Tag
) { 
    throw std::runtime_error("Sorry, joining input files with these types not implemented yet.");
}

template <typename Type, typename Tag>
class StrategistImplementation
: public Strategist<Type>
{
  public:
    typedef void result_type;
    StrategistImplementation() : Strategist<Type>(Tag::get_name(), Tag::get_desc()) {}
    StrategistImplementation* clone() const { return new StrategistImplementation(*this); }

    boost::shared_ptr< const MetaInfo > make_traits(
        const std::vector< boost::shared_ptr< const MetaInfo > >& v ) {
        boost::shared_ptr<MetaInfo> result(new MetaInfo());
        std::vector< boost::shared_ptr< const input::Traits<Type> > > traits;
        for (size_t i = 0; i < v.size(); ++i)
            if ( v[i] && v[i]->provides< Type >() ) {
                traits.push_back( v[i]->traits<Type>() );
                assert( traits.back() );
            } else
                return result;

        std::unique_ptr<input::Traits<Type>> base( merge_traits( traits, Tag() ).release() );
        if ( ! base.get() ) return result;
        result.reset(new MetaInfo(*v[0]) );
        result->set_traits( base.release() );
        return result;
    }

    std::unique_ptr<Source<Type>> make_source( typename Strategist<Type>::Sources sources ) { 
        if ( sources.size() == 0 ) return std::unique_ptr<Source<Type>>();
        return make_specialized_source( std::move(sources), Tag() );
    }
};

template <typename Type>
class Link 
: public input::Link<Type>
{
    typedef typename input::Link<Type>::Connection Connection;
    typedef typename input::Link<Type>::TraitsRef TraitsRef;

    std::vector<simparm::Object> connection_nodes;
    std::vector< std::unique_ptr<input::Link<Type>> > children;
    std::vector< Connection > connections;
    std::vector< TraitsRef > input_traits;
    simparm::Object name_object;
    simparm::TabGroup channels;
    simparm::ManagedChoiceEntry< Strategist<Type> > join_type;
    simparm::Entry<unsigned long> channel_count;
    bool registered_node;

    simparm::NodeHandle channels_node;
    simparm::BaseAttribute::ConnectionStore listening[2];

    void change_channel_count();

  public:
    Link( std::unique_ptr<input::Link<Type>> upstream );
    Link( const Link& );

  private:
    Link* clone() const OVERRIDE { return new Link(*this); }

    void traits_changed( TraitsRef, input::Link<Type>* );
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
        this->update_current_meta_info(t);
    }

    Source<Type>* makeSource() OVERRIDE;
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
        assert( this->current_meta_info().get() );
    }
};

template <typename Type>
Link<Type>::Link( std::unique_ptr<input::Link<Type>> upstream )
: name_object("MultiChannel", "Multi-channel input"),
  channels("Channels", "Channels"),
  join_type("JoinOn"),
  channel_count("ChannelCount", 1),
  registered_node(false)
{
    input_traits.push_back( upstream->current_meta_info() );
    connection_nodes.emplace_back("Channel1", "Channel 1");
    children.push_back( std::move(upstream) );
    connections.push_back( children.back()->notify(
        boost::bind( &Link::traits_changed, this, _1, children.back().get() ) ) );

    channel_count.min = 1;

    join_type.addChoice( new StrategistImplementation< Type, spatial_join::tag >() );
    join_type.addChoice( new StrategistImplementation< Type, temporal_join::tag >() );
    join_type.choose( spatial_join::tag::get_name() );
    join_type.set_visibility( false );
}

template <typename Type>
Link<Type>::Link( const Link& o )
: input::Link<Type>(o), 
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

template <typename Type>
void Link<Type>::traits_changed( TraitsRef r, input::Link<Type>* l ) {
    assert( children.size() == input_traits.size() );
    for (size_t i = 0; i < children.size(); ++i)
        if ( children[i].get() == l )
            input_traits[i] = r;
    recompute_meta_info();
}

template <typename Type>
Source<Type>* Link<Type>::makeSource() {
    if ( children.size() == 1 ) {
        return children[0]->make_source().release();
    } else {
        typename Strategist<Type>::Sources sources;
        for (auto& child : children) {
            sources.emplace_back(child->make_source());
        }
        return join_type().make_source( std::move(sources) ).release();
    }
}

template <typename Type>
void Link<Type>::change_channel_count() {
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

std::unique_ptr<input::Link<engine::ImageStack>> create_image_link(
    std::unique_ptr<input::Link<engine::ImageStack>> upstream) {
    return make_unique<Link<engine::ImageStack>>(std::move(upstream));
}

std::unique_ptr<input::Link<output::LocalizedImage>> create_localization_link(
    std::unique_ptr<input::Link<output::LocalizedImage>> upstream) {
    return make_unique<Link<output::LocalizedImage>>(std::move(upstream));
}

}
}
}
