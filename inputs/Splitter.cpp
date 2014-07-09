#include "debug.h"
#include "inputs/Splitter.h"
#include <boost/mpl/vector.hpp>
#include "engine/Image.h"
#include "engine/InputTraits.h"
#include "image/constructors.h"
#include "image/iterator.h"
#include "input/AdapterSource.h"
#include "input/FilterFactory.h"
#include "input/FilterFactoryLink.h"
#include "input/Source.h"
#include "simparm/ChoiceEntry.h"
#include "simparm/ManagedChoiceEntry.h"
#include "simparm/Entry.h"
#include "simparm/Message.h"
#include "simparm/Object.h"
#include "simparm/ObjectChoice.h"

using namespace dStorm::engine;

namespace dStorm {
namespace Splitter {

struct Split : public simparm::ObjectChoice {
    Split( std::string name, std::string desc ) : simparm::ObjectChoice(name,desc) {}
    virtual Split* clone() const = 0;
    virtual input::Source<engine::ImageStack>* make_source
        ( std::unique_ptr< input::Source<engine::ImageStack> > p ) = 0;
    virtual int split_dimension() const = 0;
};

struct Config 
{
    simparm::Object name_object;
    simparm::ManagedChoiceEntry<Split> biplane_split;
    Config();
    void attach_ui(simparm::NodeHandle at ) { biplane_split.attach_ui( name_object.attach_ui(at) ); }
};

class Source 
: public input::AdapterSource<engine::ImageStack>,
  boost::noncopyable
{
    const int splitdim;

    void modify_traits( input::Traits<engine::ImageStack>& );
    void attach_local_ui_( simparm::NodeHandle ) {}
  public:
    Source(bool vertical, std::unique_ptr< input::Source<engine::ImageStack> > base);

    bool GetNext(int thread, engine::ImageStack* target) OVERRIDE;
};

struct NoSplit : public Split {
    Split* clone() const { return new NoSplit(*this); }
    input::Source<engine::ImageStack>* make_source
        ( std::unique_ptr< input::Source<engine::ImageStack> > p ) 
        { return p.release(); }
    int split_dimension() const { return -1; }
    void attach_ui( simparm::NodeHandle at ) { attach_parent(at); }

    NoSplit() : Split("None", "None") {}
};

struct HorizontalSplit : public Split {
    Split* clone() const { return new HorizontalSplit(*this); }
    input::Source<engine::ImageStack>* make_source
        ( std::unique_ptr< input::Source<engine::ImageStack> > p ) 
        { return new Source( false, std::move(p) ); }
    int split_dimension() const { return 0; }
    void attach_ui( simparm::NodeHandle at ) { attach_parent(at); }

    HorizontalSplit() : Split("Horizontally", "Left and right") {}
};

struct VerticalSplit : public Split {
    Split* clone() const { return new VerticalSplit(*this); }
    input::Source<engine::ImageStack>* make_source
        ( std::unique_ptr< input::Source<engine::ImageStack> > p ) 
        { return new Source( true, std::move(p) ); }
    int split_dimension() const { return 1; }
    void attach_ui( simparm::NodeHandle at ) { attach_parent(at); }

    VerticalSplit() : Split("Vertically", "Top and bottom") {}
};

class SourceFactory
: public input::FilterFactory<engine::ImageStack>
{
    std::unique_ptr<input::Source<engine::ImageStack>>
    make_source( std::unique_ptr< input::Source<engine::ImageStack> > p ) OVERRIDE {
        return std::unique_ptr<input::Source<engine::ImageStack>>(
            config.biplane_split().make_source( std::move(p) ));
    }

    boost::shared_ptr<const input::Traits<engine::ImageStack>> make_meta_info(
        input::MetaInfo&,
        boost::shared_ptr<const input::Traits<engine::ImageStack>> t) OVERRIDE {
        if (!t) { return t; }

        boost::shared_ptr<input::Traits<engine::ImageStack>> n(
            new input::Traits<engine::ImageStack>(*t));
        int split_dim = config.biplane_split().split_dimension();
        if ( split_dim >= 0 ) split_planes( *n, split_dim );
        return n;
    }

    void attach_ui(simparm::NodeHandle at,
                   std::function<void()> traits_change_callback) OVERRIDE { 
        listening = config.biplane_split.value.notify_on_value_change(traits_change_callback);
        config.attach_ui( at ); 
    }

    SourceFactory* clone() const { return new SourceFactory(*this); }

    Config config;
    simparm::BaseAttribute::ConnectionStore listening;

  public:

    static void split_planes( input::Traits<engine::ImageStack>& t, int dim )
    {
        input::Traits<engine::ImageStack> old( t );
        t.clear();

        for (int i = 0; i < old.plane_count(); ++i) {
            engine::InputPlane plane( old.plane(i) );
            plane.image.size[dim] /= 2;
            for (int c = 0; c < 2; ++c)
                t.push_back( plane );
        }
    }
};

Config::Config() 
: name_object( "BiplaneSplitter", "Split dual view image"),
  biplane_split("DualView")
{
    biplane_split.addChoice( std::auto_ptr<Split>( new NoSplit() ) );
    biplane_split.addChoice( std::auto_ptr<Split>( new HorizontalSplit() ) );
    biplane_split.addChoice( std::auto_ptr<Split>( new VerticalSplit() ) );

    biplane_split.set_user_level( simparm::Intermediate );
}

Source::Source(bool vertical, std::unique_ptr<input::Source<engine::ImageStack> > base)
: input::AdapterSource<engine::ImageStack>(std::move(base)), splitdim(vertical ? 1 : 0) {
}

void Source::modify_traits( input::Traits<engine::ImageStack>& s ) {
    SourceFactory::split_planes( s, splitdim );
}

bool Source::GetNext(int thread, engine::ImageStack* result) {
    engine::ImageStack e;
    if (!input::AdapterSource<engine::ImageStack>::GetNext(thread, &e)) {
        return false;
    }

    *result = engine::ImageStack( e.frame_number() );
    for (int p = 0; p < e.plane_count(); ++p ) {
        if ( e.plane(p).is_invalid() ) {
            for (int j = 0; j < 2; ++j) {
                result->push_back( e.plane(p) );
            }
        } else {
            const engine::Image2D& im = e.plane(p);
            engine::Image2D::Size sz = im.sizes();
            engine::Image2D::Offsets o = im.get_offsets();
            sz[splitdim] /= 2;
            const int offset = sz[splitdim].value() * o[splitdim];
            for (int j = 0; j < 2; ++j) {
                result->push_back(engine::Image2D( 
                        sz, im.get_data_reference(), o,
                        im.get_global_offset() + j * offset, 
                        im.frame_number() ) );
            }
        }
    }
    return true;
}

std::auto_ptr<input::Link> makeLink() {
    return CreateLink(std::unique_ptr<SourceFactory>(new SourceFactory()));
}

}
}
