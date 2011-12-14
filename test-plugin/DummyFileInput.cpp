#define BOOST_DISABLE_ASSERTS
#include <dStorm/namespaces.h>
#include <dStorm/log.h>
#include "DummyFileInput.h"
#include <iostream>
#include <fstream>
#include <dStorm/engine/Image.h>
#include <dStorm/image/constructors.h>
#include <boost/iterator/iterator_facade.hpp>
#include <dStorm/input/InputFileNameChange.h>
#include <dStorm/input/InputMutex.h>

#undef DEBUG
#define VERBOSE
#include <dStorm/log.h>

using namespace dStorm::input;
using namespace dStorm::input::chain;
using namespace boost::units;

namespace dummy_file_input {

Source::Source(const Config& config, boost::shared_ptr<OpenFile> of) 
: simparm::Set("YDummyInput", "Dummy input"),
  dStorm::input::Source<dStorm::engine::Image>
    ( static_cast<simparm::Node&>(*this), Capabilities() ),
  of( of )
{
}

Source::~Source() {}

Source::TraitsPtr
Source::get_traits()
{
    return TraitsPtr( dynamic_cast< Traits* >(of->getTraits().release()) );
}

class Source::_iterator 
: public boost::iterator_facade<_iterator,dStorm::engine::Image,std::input_iterator_tag>
{
    dStorm::engine::Image::Size sz;
    mutable dStorm::engine::Image image;
    int n;

    friend class boost::iterator_core_access;

    dStorm::engine::Image& dereference() const { DEBUG( "Image number " << n << " is accessed" ); return image; }
    void increment() { n++; image = dStorm::engine::Image(sz, n * boost::units::camera::frame); }
    bool equal(const _iterator& o) const { return o.n == n; }

  public:
    _iterator(int pos, const OpenFile& of) 
        : sz(of.get_size()), image(sz), n(pos) { image.fill(0); }
};

Source::iterator Source::begin() {
    return iterator( _iterator(0, *of) );
}
Source::iterator Source::end() {
    return iterator( _iterator(of->image_number(), *of) );
}

Config::Config()
: simparm::Object("DummyInput", "Dummy file input driver"),
  width("Width", "Image width", 25),
  height("Height", "Image height", 50),
  number("Number", "Number of generated images", 10),
  goIntType("GoIntType", "Make input source of type int", false)
{}

void Config::registerNamedEntries() {
    push_back( width );
    push_back( height );
    push_back( number );
    push_back( goIntType );
}

Method::Method() 
    //"extension_dum", ".dum")
: simparm::Listener( simparm::Event::ValueChanged )
{
    registerNamedEntries();
}

Source* Method::makeSource() {
    return new Source(config, get_file());
}

void Method::operator()(const simparm::Event& e) {
    ost::MutexLock lock( global_mutex() );
    reread_file();
}

void Method::registerNamedEntries() {
    receive_changes_from( config.width.value );
    receive_changes_from( config.height.value );
    receive_changes_from( config.number.value );
    receive_changes_from( config.goIntType.value );
}

}
