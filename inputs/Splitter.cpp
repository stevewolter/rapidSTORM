#include "debug.h"
#include "Splitter.h"
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/mpl/vector.hpp>
#include <dStorm/engine/Image.h>
#include <dStorm/image/constructors.h>
#include <dStorm/image/iterator.h>
#include <dStorm/input/AdapterSource.h>
#include <dStorm/input/InputMutex.h>
#include <dStorm/input/Method.hpp>
#include <dStorm/input/Source.h>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/ManagedChoiceEntry.hh>
#include <simparm/Entry.hh>
#include <simparm/Message.hh>
#include <simparm/Object.hh>
#include <simparm/Structure.hh>
#include <dStorm/make_clone_allocator.hpp>

using namespace dStorm::engine;

namespace dStorm {
namespace Splitter {

struct Split {
    virtual ~Split() {}
    virtual Split* clone() const = 0;
    virtual input::Source<engine::ImageStack>* make_source
        ( std::auto_ptr< input::Source<engine::ImageStack> > p ) = 0;
    virtual int split_dimension() const = 0;
    virtual simparm::Node& getNode() = 0;
    virtual const simparm::Node& getNode() const = 0;
};

}
}

DSTORM_MAKE_BOOST_CLONE_ALLOCATOR(dStorm::Splitter::Split)

namespace dStorm {
namespace Splitter {


struct Config : public simparm::Object
{
    simparm::ManagedChoiceEntry<Split> biplane_split;
    Config();
    void registerNamedEntries() { push_back(biplane_split); }
};

class Source 
: public input::AdapterSource<engine::ImageStack>,
  boost::noncopyable
{
    struct iterator;
    const bool vertical;

    void modify_traits( input::Traits<engine::ImageStack>& );
  public:
    Source(bool vertical, std::auto_ptr< input::Source<engine::ImageStack> > base);

    input::Source<engine::ImageStack>::iterator begin();
    input::Source<engine::ImageStack>::iterator end();
};

struct NoSplit : public Split {
    Split* clone() const { return new NoSplit(*this); }
    input::Source<engine::ImageStack>* make_source
        ( std::auto_ptr< input::Source<engine::ImageStack> > p ) 
        { return p.release(); }
    int split_dimension() const { return -1; }
    simparm::Node& getNode() { return node; }
    const simparm::Node& getNode() const { return node; }

    simparm::Object node;
    NoSplit() : node("None", "None") {}
};

struct HorizontalSplit : public Split {
    Split* clone() const { return new HorizontalSplit(*this); }
    input::Source<engine::ImageStack>* make_source
        ( std::auto_ptr< input::Source<engine::ImageStack> > p ) 
        { return new Source( false, p ); }
    int split_dimension() const { return 0; }
    simparm::Node& getNode() { return node; }
    const simparm::Node& getNode() const { return node; }

    simparm::Object node;
    HorizontalSplit() : node("Horizontally", "Left and right") {}
};

struct VerticalSplit : public Split {
    Split* clone() const { return new VerticalSplit(*this); }
    input::Source<engine::ImageStack>* make_source
        ( std::auto_ptr< input::Source<engine::ImageStack> > p ) 
        { return new Source( true, p ); }
    int split_dimension() const { return 1; }
    simparm::Node& getNode() { return node; }
    const simparm::Node& getNode() const { return node; }

    simparm::Object node;
    VerticalSplit() : node("Vertically", "Top and bottom") {}
};

class ChainLink
: public input::Method<ChainLink>, public simparm::Listener
{
    friend class input::Method<ChainLink>;
    typedef boost::mpl::vector< dStorm::engine::ImageStack > SupportedTypes;

    bool ignore_unknown_type() const { return true; }

    input::Source<engine::ImageStack>* make_source( std::auto_ptr< input::Source<engine::ImageStack> > p ) {
        return config.biplane_split().make_source( p );
    }

    void update_traits( input::MetaInfo&, input::Traits<engine::ImageStack>& t ) {
        int split_dim = config.biplane_split().split_dimension();
        if ( split_dim >= 0 ) split_planes( t, split_dim );
    }

    simparm::Structure<Config>& get_config() { return config; }
    simparm::Structure<Config> config;
    void operator()( const simparm::Event& );
  public:
    ChainLink();
    ChainLink(const ChainLink&);

    simparm::Node& getNode() { return config; }
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
: simparm::Object("BiplaneSplitter", "Split dual view image"),
  biplane_split("DualView", "Dual view")
{
    biplane_split.addChoice( std::auto_ptr<Split>( new NoSplit() ) );
    biplane_split.addChoice( std::auto_ptr<Split>( new HorizontalSplit() ) );
    biplane_split.addChoice( std::auto_ptr<Split>( new VerticalSplit() ) );

    biplane_split.userLevel = simparm::Object::Intermediate;
}

Source::Source(bool vertical, std::auto_ptr<input::Source<engine::ImageStack> > base)
: input::AdapterSource<engine::ImageStack>(base), vertical(vertical)
{
}

void Source::modify_traits( input::Traits<engine::ImageStack>& s ) {
    const int dim = (vertical) ? 1 : 0;
    ChainLink::split_planes( s, dim );
}

struct Source::iterator 
: public boost::iterator_adaptor<iterator, input::Source<engine::ImageStack>::iterator>
{
    const int splitdim;
    mutable boost::optional<engine::ImageStack> i;

    iterator( bool vertical, input::Source<engine::ImageStack>::iterator base )
        :  iterator::iterator_adaptor_(base), splitdim( (vertical) ? 1 : 0) {}
  private:
    friend class boost::iterator_core_access;
    void increment() { ++this->base_reference(); i.reset(); }
    engine::ImageStack& dereference() const; 
};

engine::ImageStack& Source::iterator::dereference() const {
    if ( ! i.is_initialized() ) {
        const engine::ImageStack& e = *base();
        DEBUG("Upstream has frame number " << e.frame_number());
        i = engine::ImageStack( e.frame_number() );
        for (int p = 0; p < e.plane_count(); ++p ) {
            if ( e.plane(p).is_invalid() ) {
                for (int j = 0; j < 2; ++j)
                    i->push_back( e.plane(p) );
            } else {
                const engine::Image2D& im = e.plane(p);
                engine::Image2D::Size sz = im.sizes();
                engine::Image2D::Offsets o = im.get_offsets();
                sz[splitdim] /= 2;
                const int offset = sz[splitdim].value() * o[splitdim];
                for (int j = 0; j < 2; ++j) {
                    i->push_back( 
                        engine::Image2D( 
                            sz, im.get_data_reference(), o,
                            im.get_global_offset() + j * offset, 
                            im.frame_number() ) );
                }
            }
        }
    }
    DEBUG("Result has frame number " << i->frame_number());
    return *i;
}

input::Source<engine::ImageStack>::iterator
Source::begin() {
    return input::Source<engine::ImageStack>::iterator( iterator(vertical, base().begin()) );
}

input::Source<engine::ImageStack>::iterator
Source::end() {
    return input::Source<engine::ImageStack>::iterator( iterator(vertical, base().end()) );
}

void ChainLink::operator()( const simparm::Event& ) {
    input::InputMutexGuard lock( input::global_mutex() );
    republish_traits();
}

ChainLink::ChainLink()
: simparm::Listener( simparm::Event::ValueChanged )
{
    receive_changes_from( config.biplane_split.value );
}

ChainLink::ChainLink(const ChainLink& o)
: input::Method<ChainLink>(o), simparm::Listener( simparm::Event::ValueChanged ),
  config(o.config)
{
    receive_changes_from( config.biplane_split.value );
}

std::auto_ptr<input::Link> makeLink() {
    return std::auto_ptr<input::Link>( new ChainLink() );
}

}
}
