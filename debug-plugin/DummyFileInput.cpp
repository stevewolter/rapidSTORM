#include "DummyFileInput.h"
#include <iostream>
#include <dStorm/engine/Image.h>
#include <boost/iterator/iterator_facade.hpp>
#include <dStorm/input/FileBasedMethod_impl.h>

using namespace dStorm::input;

namespace dummy_file_input {

Source::Source(const Method& config) 
: simparm::Set("YDummyInput", "Dummy input"),
  dStorm::input::Source<dStorm::engine::Image>
    ( static_cast<simparm::Node&>(*this), Capabilities() ),
  number( config.number() )
{
    size.x() = config.width() * cs_units::camera::pixel;
    size.y() = config.height() * cs_units::camera::pixel;
    std::cout << "Simulating file input from '" << config.inputFile().c_str() << "'" << std::endl;
}

Source::~Source() {}

Source::TraitsPtr
Source::get_traits()
{
    TraitsPtr rv( new TraitsPtr::element_type() );
    rv->size = size;
    rv->last_frame = (number - 1) * cs_units::camera::frame;
    return rv;
}

class Source::_iterator 
: public boost::iterator_facade<_iterator,dStorm::engine::Image,std::input_iterator_tag>
{
    mutable dStorm::engine::Image image;
    int n;

    friend class boost::iterator_core_access;

    dStorm::engine::Image& dereference() const { return image; }
    void increment() { n++; image.frame_number() = n * cs_units::camera::frame; }
    bool equal(const _iterator& o) const { return o.n == n; }

  public:
    _iterator(int pos, dStorm::engine::Image::Size sz) 
        : image(sz), n(pos) {}
};

Source::iterator Source::begin() {
    return iterator( _iterator(0, size) );
}
Source::iterator Source::end() {
    return iterator( _iterator(number, size) );
}

Config::Config()
: width("Width", "Image width", 25),
  height("Height", "Image height", 50),
  number("Number", "Number of generated images", 10)
{}

void Method::registerNamedEntries() {
    push_back( width );
    push_back( height );
    push_back( number );
}

Method::Method(dStorm::input::Config& c) 
: dStorm::input::FileBasedMethod<dStorm::engine::Image>(c,
    "DummyInput", "Dummy file input driver",
    "extension_dum", ".dum")
{
    registerNamedEntries();
}

Method::Method(const Method& f, dStorm::input::Config& c) 
: Config(f),
  dStorm::input::FileBasedMethod<dStorm::engine::Image>(f,c)
{
    registerNamedEntries();
}

}
