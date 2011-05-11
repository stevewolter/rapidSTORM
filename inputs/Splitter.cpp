#include "debug.h"
#include "Splitter.h"
#include <dStorm/input/Source_impl.h>
#include <dStorm/engine/Image.h>
#include <dStorm/image/constructors.h>
#include <dStorm/image/iterator.h>
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/Filter_impl.h>
#include <simparm/Message.hh>
#include <boost/iterator/iterator_adaptor.hpp>
#include <dStorm/ImageTraits.h>

using namespace dStorm::engine;

namespace dStorm {

namespace input {
namespace chain {

template <>
template <typename Type>
bool DefaultVisitor<Splitter::Config>::operator()( input::Traits<Type>& t ) {
    if ( this->config.enable() ) {
        t.size[2] = 2 * camera::pixel;
        t.size[1] /= 2;
        t.planes.push_back( t.plane(0) );
    } else {
    }
    return true;
}

template <>
bool DefaultVisitor<Splitter::Config>::operator()( Context& c ) {
    return true;
}

template <>
template <typename Type>
bool DefaultVisitor<Splitter::Config>::operator()( std::auto_ptr< input::Source<Type> > p ) {
    new_source.reset( new Splitter::Source(p) );
    return true;
}

}
}

namespace Splitter {

Config::Config() 
: simparm::Object("BiplaneSplitter", "Split dual view image"),
  enable("DualView", "Input is dual view")
{
}

Source::Source(std::auto_ptr<input::Source<engine::Image> > base)
: input::Source<engine::Image>(base->getNode(), base->flags),
  base(base)
{
}

Source::TraitsPtr 
Source::get_traits()
{
    DEBUG("Running background standard deviation estimation");
    Source::TraitsPtr s = base->get_traits();
    s->size[1] /= 2;
    s->size[2] *= 2;
    return s;
}

struct Source::iterator 
: public boost::iterator_adaptor<iterator, input::Source<engine::Image>::iterator>
{
    mutable engine::Image i;

    iterator( input::Source<engine::Image>::iterator base )
        : iterator::iterator_adaptor_(base) {}
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
        sz[1] /= 2;
        sz[2] *= 2;
        o[2] = sz[1].value() * o[1];
        i = engine::Image( sz, e.get_data_reference(), o, e.get_global_offset(), e.frame_number() );
    }
    return i;
}

input::Source<engine::Image>::iterator
Source::begin() {
    return input::Source<engine::Image>::iterator( iterator(base->begin()) );
}

input::Source<engine::Image>::iterator
Source::end() {
    return input::Source<engine::Image>::iterator( iterator(base->end()) );
}

input::chain::Link::AtEnd
ChainLink::traits_changed( ChainLink::TraitsRef r, Link* l) {
    last_traits = r;
    return input::chain::DelegateToVisitor::traits_changed(*this,r,l);
}

input::chain::Link::AtEnd
ChainLink::context_changed( ChainLink::ContextRef c, Link* l)
{
    return input::chain::DelegateToVisitor::context_changed(*this,c,l);
}

input::BaseSource* ChainLink::makeSource()
{
    if ( ! config.enable() )
        return Forwarder::makeSource();
    else
        return input::chain::DelegateToVisitor::makeSource(*this);
}

void ChainLink::operator()( const simparm::Event& ) {
    if ( last_traits.get() ) {
        input::chain::DelegateToVisitor::traits_changed(*this, last_traits, NULL);
    }
}

ChainLink::ChainLink()
: simparm::Listener( simparm::Event::ValueChanged )
{
    receive_changes_from( config.enable.value );
}

ChainLink::ChainLink(const ChainLink& o)
: Filter(o), simparm::Listener( simparm::Event::ValueChanged ),
  last_traits(o.last_traits), config(o.config)
{
    receive_changes_from( config.enable.value );
}

std::auto_ptr<input::chain::Filter> makeLink() {
    return std::auto_ptr<input::chain::Filter>( new ChainLink() );
}

}
}
