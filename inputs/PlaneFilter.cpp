#include "debug.h"
#include "PlaneFilter.h"

#include <simparm/BoostUnits.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Entry_Impl.hh>
#include <simparm/Structure.hh>
#include <boost/lexical_cast.hpp>
#include <boost/optional/optional.hpp>
#include <boost/units/io.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant.hpp>
#include <dStorm/image/extend.h>
#include <dStorm/Image.h>
#include <dStorm/image/slice.h>
#include <dStorm/input/AdapterSource.h>
#include <dStorm/input/MetaInfo.h>
#include <dStorm/input/InputMutex.h>
#include <dStorm/localization/Traits.h>
#include <dStorm/input/Method.hpp>
#include <dStorm/UnitEntries/FrameEntry.h>
#include <dStorm/units/frame_count.h>

namespace dStorm {
namespace plane_filter {

struct Config : public simparm::Object {
    simparm::ChoiceEntry which_plane;

    Config();
    void registerNamedEntries() { 
        push_back( which_plane );
    }
};

class Source
: public input::AdapterSource< engine::ImageStack >
{
    const int plane;
    typedef input::Source< engine::ImageStack >::iterator base_iterator;

    struct _iterator;

  public:
    Source( std::auto_ptr< input::Source<engine::ImageStack> > upstream,
            int plane)
        : input::AdapterSource<engine::ImageStack>(upstream), plane(plane) {}
    Source* clone() const { throw std::logic_error("Not implemented"); }

    base_iterator begin();
    base_iterator end();
    void modify_traits( input::Traits<engine::ImageStack>& p)
    {
        engine::InputPlane only = p.plane(plane);
        p.clear();
        p.push_back( only );
    }
};

class Source::_iterator 
  : public boost::iterator_adaptor< _iterator, base_iterator >
{
    int plane;
    mutable bool is_initialized;
    mutable engine::ImageStack i;

    friend class boost::iterator_core_access;
    void increment() { 
        ++this->base_reference(); 
    }

    void select_plane();

    engine::ImageStack& dereference() const { 
        if ( ! is_initialized ) {
            const engine::ImageStack& all_planes = *this->base();
            i = engine::ImageStack( all_planes.frame_number() );
            i.push_back( all_planes.plane( plane ) );
        }
        return i; 
    }
    
  public:
    explicit _iterator(int plane, const base_iterator base)
      : _iterator::iterator_adaptor_(base), plane(plane) {}
};

Source::base_iterator Source::begin() { 
    return base_iterator( _iterator( plane, this->base().begin() ) ); 
}

Source::base_iterator Source::end() {
    return base_iterator( _iterator( plane, this->base().end() ) ); 
}

class ChainLink 
: public input::Method<ChainLink>, public simparm::Listener
{
    friend class input::Method<ChainLink>;

    simparm::Structure<Config> config;
    simparm::Structure<Config>& get_config() { return config; }
    void operator()( const simparm::Event& );

    typedef Localization::ImageNumber::Traits TemporalTraits;

    void update_traits( input::MetaInfo&, input::Traits<engine::ImageStack>& traits ) {
        if ( config.which_plane() != -1 ) {
            engine::InputPlane only = traits.plane( config.which_plane() );
            traits.clear();
            traits.push_back( only );
        }
    }
    template <typename Type>
    void update_traits( input::MetaInfo&, input::Traits<Type>& traits ) { 
    }

    void notice_traits( const input::MetaInfo&, const input::Traits<engine::ImageStack>& t ) {
        config.which_plane.viewable = true;
        for (int i = config.which_plane.numChoices()-1; i < t.plane_count(); ++i) {
            std::string id = boost::lexical_cast<std::string>(i);
            config.which_plane.addChoice( i, "Plane" + id, "Plane " + id );
        }
        for (int i = t.plane_count(); i < config.which_plane.numChoices()-1; ++i)
            config.which_plane.removeChoice( i );
    }
    template <typename Type>
    void notice_traits( const input::MetaInfo&, const input::Traits<Type>& ) {
        config.which_plane.viewable = false;
    }

    template <typename Type>
    input::Source<Type>* make_source( std::auto_ptr< input::Source<Type> > p ) 
        { return p.release(); }
    input::Source<engine::ImageStack>* make_source( std::auto_ptr< input::Source<engine::ImageStack> > p ) {
        if ( config.which_plane() != -1 )
            return new Source( p, config.which_plane() );
        else
            return p.release();
    }

  public:
    ChainLink();
    ChainLink(const ChainLink&);
    simparm::Node& getNode() { return config; }
};

Config::Config() 
: simparm::Object("PlaneFilter", "Image selection filter"),
  which_plane( "OnlyPlane", "Process only one plane" )
{
    which_plane.addChoice(-1, "AllPlanes", "All planes");
    which_plane.userLevel = simparm::Object::Expert;
}

void ChainLink::operator()( const simparm::Event& ) {
    input::InputMutexGuard lock( input::global_mutex() );
    republish_traits();
}

ChainLink::ChainLink()
: simparm::Listener( simparm::Event::ValueChanged )
{
    receive_changes_from( config.which_plane.value );
}

ChainLink::ChainLink(const ChainLink& o)
: input::Method<ChainLink>(o), simparm::Listener( simparm::Event::ValueChanged ),
  config(o.config)
{
    receive_changes_from( config.which_plane.value );
}

std::auto_ptr<input::Link> make_link() {
    return std::auto_ptr<input::Link>( new ChainLink() );
}

}
}
