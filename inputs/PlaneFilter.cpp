#include "debug.h"
#include "inputs/PlaneFilter.h"

#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/optional/optional.hpp>
#include <boost/units/io.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant.hpp>

#include "helpers/make_unique.hpp"
#include "engine/InputTraits.h"
#include "image/extend.h"
#include "image/Image.h"
#include "image/slice.h"
#include "input/AdapterSource.h"
#include "input/FilterFactory.h"
#include "input/InputMutex.h"
#include "input/MetaInfo.h"
#include "localization/Traits.h"
#include "simparm/BoostUnits.h"
#include "simparm/ManagedChoiceEntry.h"
#include "simparm/ObjectChoice.h"
#include "UnitEntries/FrameEntry.h"
#include "units/frame_count.h"

namespace dStorm {
namespace plane_filter {

struct PlaneSelection : public simparm::ObjectChoice {
    PlaneSelection( std::string name, std::string desc ) : simparm::ObjectChoice(name,desc) {}
    virtual PlaneSelection* clone() const = 0;
    virtual bool selects_plane() const = 0;
    virtual int plane_index() const = 0;
};

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
    void attach_local_ui_( simparm::NodeHandle ) OVERRIDE {}
    bool GetNext(int thread, engine::ImageStack* target) OVERRIDE;

  public:
    Source( std::unique_ptr< input::Source<engine::ImageStack> > upstream,
            int plane)
        : input::AdapterSource<engine::ImageStack>(std::move(upstream)), plane(plane) {}

    void modify_traits( input::Traits<engine::ImageStack>& p) OVERRIDE {
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

class Factory : public input::FilterFactory<engine::ImageStack>
{
  public:
    Factory* clone() const OVERRIDE { return new Factory(*this); }

    void attach_ui( simparm::NodeHandle at,
                    std::function<void()> traits_change_callback) OVERRIDE { 
        listening = config.which_plane.value.notify_on_value_change(
                traits_change_callback);
        config.attach_ui( at ); 
    }

    std::unique_ptr<input::Source<engine::ImageStack>> make_source(
        std::unique_ptr<input::Source<engine::ImageStack>> p) OVERRIDE {
        if ( config.which_plane().selects_plane() )
            return make_unique<Source>( std::move(p), config.which_plane().plane_index() );
        else
            return std::move(p);
    }

    boost::shared_ptr<const input::Traits<engine::ImageStack>> make_meta_info(
        input::MetaInfo& meta_info,
        boost::shared_ptr<const input::Traits<engine::ImageStack>> t) OVERRIDE {
        config.which_plane.show();
        for (int i = config.which_plane.size()-1; i < t->plane_count(); ++i) {
            config.which_plane.addChoice( new SinglePlane(i) );
        }
        for (int i = t->plane_count(); i < config.which_plane.size()-1; ++i)
            config.which_plane.removeChoice( i );

        if ( config.which_plane().selects_plane() ) {
            auto mine = boost::make_shared<input::Traits<engine::ImageStack>>(*t);
            engine::InputPlane only = t->plane( config.which_plane().plane_index() );
            mine->clear();
            mine->push_back( only );
            return mine;
        } else {
            return t;
        }
    }

  private:
    Config config;
    simparm::BaseAttribute::ConnectionStore listening;

};

Config::Config() 
: name_object( "PlaneFilter", "Image selection filter"),
  which_plane( "OnlyPlane" )
{
    which_plane.addChoice( new AllPlanes() );
    which_plane.set_user_level( simparm::Expert );
}

std::unique_ptr<input::FilterFactory<engine::ImageStack>> create() {
    return make_unique<Factory>();
}

}
}
