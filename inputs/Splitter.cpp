#include "debug.h"
#include "Splitter.h"
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/mpl/vector.hpp>
#include <dStorm/engine/Image.h>
#include <dStorm/image/constructors.h>
#include <dStorm/image/iterator.h>
#include <dStorm/ImageTraits.h>
#include <dStorm/input/AdapterSource.h>
#include <dStorm/input/InputMutex.h>
#include <dStorm/input/Method.hpp>
#include <dStorm/input/Source.h>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Entry.hh>
#include <simparm/Message.hh>
#include <simparm/Object.hh>
#include <simparm/Structure.hh>

using namespace dStorm::engine;

namespace dStorm {
namespace Splitter {

struct Config : public simparm::Object
{
    enum Splits { Horizontal, Vertical, None };

    simparm::ChoiceEntry biplane_split;
    Config();
    void registerNamedEntries() { push_back(biplane_split); }
};

class Source 
: public input::AdapterSource<engine::Image>,
  boost::noncopyable
{
    struct iterator;
    const bool vertical;

    void modify_traits( input::Traits<engine::Image>& );
  public:
    Source(bool vertical, std::auto_ptr< input::Source<engine::Image> > base);

    input::Source<engine::Image>::iterator begin();
    input::Source<engine::Image>::iterator end();
};


class ChainLink
: public input::Method<ChainLink>, public simparm::Listener
{
    friend class input::Method<ChainLink>;
    typedef boost::mpl::vector< dStorm::engine::Image > SupportedTypes;

    bool ignore_unknown_type() const { return true; }

    input::Source<engine::Image>* make_source( std::auto_ptr< input::Source<engine::Image> > p ) {
        switch( config.biplane_split() ) {
            case Config::Vertical: return new Source( true, p );
            case Config::Horizontal: return new Source( false, p );
            case Config::None: return p.release();
            default: throw std::logic_error("Case fall-through");
        }
    }

    void update_traits( input::MetaInfo&, input::Traits<engine::Image>& t ) {
        if ( config.biplane_split() != Config::None ) {
            t.size[2] = 2 * camera::pixel;
            if ( config.biplane_split() == Config::Vertical )
                t.size[1] /= 2;
            else
                t.size[0] /= 2;
            t.planes.push_back( t.plane(0) );
        }
    }

    simparm::Structure<Config>& get_config() { return config; }
    simparm::Structure<Config> config;
    void operator()( const simparm::Event& );
  public:
    ChainLink();
    ChainLink(const ChainLink&);

    simparm::Node& getNode() { return config; }
};

Config::Config() 
: simparm::Object("BiplaneSplitter", "Split dual view image"),
  biplane_split("DualView", "Dual view")
{
    biplane_split.addChoice( None, "None", "None" );
    biplane_split.addChoice( Horizontal, "Horizontally", "Left and right" );
    biplane_split.addChoice( Vertical, "Vertically", "Top and bottom" );
    biplane_split.userLevel = simparm::Object::Intermediate;
}

Source::Source(bool vertical, std::auto_ptr<input::Source<engine::Image> > base)
: input::AdapterSource<engine::Image>(base), vertical(vertical)
{
}

void Source::modify_traits( input::Traits<engine::Image>& s ) {
    DEBUG("Running background standard deviation estimation");
    if ( vertical )
        s.size[1] /= 2;
    else
        s.size[0] /= 2;
    s.size[2] *= 2;
    s.planes.resize( s.size[2].value(), traits::Optics<2>() );
}

struct Source::iterator 
: public boost::iterator_adaptor<iterator, input::Source<engine::Image>::iterator>
{
    const int splitdim;
    mutable engine::Image i;

    iterator( bool vertical, input::Source<engine::Image>::iterator base )
        :  iterator::iterator_adaptor_(base), splitdim( (vertical) ? 1 : 0) {}
  private:
    friend class boost::iterator_core_access;
    void increment() { ++this->base_reference(); i.invalidate(); }
    engine::Image& dereference() const; 
};

engine::Image& Source::iterator::dereference() const {
    if ( i.is_invalid() && base()->is_valid() ) {
        const engine::Image& e = *base();
        engine::Image::Size sz = e.sizes();
        engine::Image::Offsets o = e.get_offsets();
        sz[splitdim] /= 2;
        sz[2] *= 2;
        o[2] = sz[splitdim].value() * o[splitdim];
        i = engine::Image( sz, e.get_data_reference(), o, e.get_global_offset(), e.frame_number() );
    }
    return i;
}

input::Source<engine::Image>::iterator
Source::begin() {
    return input::Source<engine::Image>::iterator( iterator(vertical, base().begin()) );
}

input::Source<engine::Image>::iterator
Source::end() {
    return input::Source<engine::Image>::iterator( iterator(vertical, base().end()) );
}

void ChainLink::operator()( const simparm::Event& ) {
    boost::lock_guard<boost::mutex> lock( input::global_mutex() );
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
