#include "debug.h"
#include "PlaneFilter.h"

#include <simparm/BoostUnits.h>
#include <simparm/ObjectChoice.h>
#include <simparm/ManagedChoiceEntry.h>
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
#include <dStorm/make_clone_allocator.hpp>

namespace dStorm {
namespace plane_filter {

struct PlaneSelection : public simparm::ObjectChoice {
    PlaneSelection( std::string name, std::string desc ) : simparm::ObjectChoice(name,desc) {}
    virtual PlaneSelection* clone() const = 0;
    virtual bool selects_plane() const = 0;
    virtual int plane_index() const = 0;
};

}
}

DSTORM_MAKE_BOOST_CLONE_ALLOCATOR(dStorm::plane_filter::PlaneSelection)

namespace dStorm {
namespace plane_filter {


struct AllPlanes : public PlaneSelection {
    AllPlanes() : PlaneSelection("AllPlanes", "All planes") {}
    AllPlanes* clone() const { return new AllPlanes(*this); }
    bool selects_plane() const { return false; }
    int plane_index() const { throw std::logic_error("No plane selectable"); }
    void attach_ui( simparm::NodeHandle at ) { attach_parent(at); }
};

struct SinglePlane : public PlaneSelection {
    int index;
    SinglePlane(int index) 
        : PlaneSelection("Plane" + boost::lexical_cast<std::string>(index), 
                         "Plane " + boost::lexical_cast<std::string>(index)),
          index(index) {}
    SinglePlane* clone() const { return new SinglePlane(*this); }

    bool selects_plane() const { return true; }
    int plane_index() const { return index; }
    void attach_ui( simparm::NodeHandle at ) { attach_parent(at); }
};

class Config {
    simparm::Object name_object;
public:
    simparm::ManagedChoiceEntry<PlaneSelection> which_plane;

    Config();
    void attach_ui( simparm::NodeHandle at ) { 
        which_plane.attach_ui( name_object.attach_ui(at) );
    }
};

class Source
: public input::AdapterSource< engine::ImageStack >
{
    const int plane;
    void attach_local_ui_( simparm::NodeHandle ) {}
    bool GetNext(int thread, engine::ImageStack* target) OVERRIDE;

  public:
    Source( std::auto_ptr< input::Source<engine::ImageStack> > upstream,
            int plane)
        : input::AdapterSource<engine::ImageStack>(upstream), plane(plane) {}
    Source* clone() const { throw std::logic_error("Not implemented"); }

    void modify_traits( input::Traits<engine::ImageStack>& p)
    {
        engine::InputPlane only = p.plane(plane);
        p.clear();
        p.push_back( only );
    }
};

bool Source::GetNext(int thread, engine::ImageStack* target) {
    engine::ImageStack all_planes;
    if (!input::AdapterSource< engine::ImageStack >::GetNext(thread, &all_planes)) {
        return false;
    }

    *target = engine::ImageStack( all_planes.frame_number() );
    target->push_back( all_planes.plane( plane ) );
    return true;
}

class ChainLink 
: public input::Method<ChainLink>
{
    friend class input::Method<ChainLink>;

    Config config;
    simparm::BaseAttribute::ConnectionStore listening;

    void update_traits( input::MetaInfo&, input::Traits<engine::ImageStack>& traits ) {
        if ( config.which_plane().selects_plane() ) {
            engine::InputPlane only = traits.plane( config.which_plane().plane_index() );
            traits.clear();
            traits.push_back( only );
        }
    }
    template <typename Type>
    void update_traits( input::MetaInfo&, input::Traits<Type>& traits ) { 
    }

    void notice_traits( const input::MetaInfo&, const input::Traits<engine::ImageStack>& t ) {
        config.which_plane.show();
        for (int i = config.which_plane.size()-1; i < t.plane_count(); ++i) {
            config.which_plane.addChoice( new SinglePlane(i) );
        }
        for (int i = t.plane_count(); i < config.which_plane.size()-1; ++i)
            config.which_plane.removeChoice( i );
    }
    template <typename Type>
    void notice_traits( const input::MetaInfo&, const input::Traits<Type>& ) {
        config.which_plane.hide();
    }

    template <typename Type>
    input::Source<Type>* make_source( std::auto_ptr< input::Source<Type> > p ) 
        { return p.release(); }
    input::Source<engine::ImageStack>* make_source( std::auto_ptr< input::Source<engine::ImageStack> > p ) {
        if ( config.which_plane().selects_plane() )
            return new Source( p, config.which_plane().plane_index() );
        else
            return p.release();
    }

  public:
    void attach_ui( simparm::NodeHandle at ) { 
        listening = config.which_plane.value.notify_on_value_change( 
            boost::bind( &input::Method<ChainLink>::republish_traits_locked, this ) );
        config.attach_ui( at ); 
    }
    static std::string getName() { return "PlaneFilter"; }
};

Config::Config() 
: name_object( ChainLink::getName(), "Image selection filter"),
  which_plane( "OnlyPlane" )
{
    which_plane.addChoice( new AllPlanes() );
    which_plane.set_user_level( simparm::Expert );
}

std::auto_ptr<input::Link> make_link() {
    return std::auto_ptr<input::Link>( new ChainLink() );
}

}
}
