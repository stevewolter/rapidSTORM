#include "DummyFileInput.h"
#include <iostream>
#include <dStorm/engine/Image.h>
#include <boost/iterator/iterator_facade.hpp>
#include <dStorm/input/FileContext.h>

using namespace dStorm::input;

namespace dummy_file_input {

Source::Source(const Config& config, dStorm::input::Context::Ptr ptr) 
: simparm::Set("YDummyInput", "Dummy input"),
  dStorm::input::Source<dStorm::engine::Image>
    ( static_cast<simparm::Node&>(*this), Capabilities() ),
  number( config.number() )
{
    FileContext& context = static_cast<FileContext&>(*ptr);
    size.x() = config.width() * cs_units::camera::pixel;
    size.y() = config.height() * cs_units::camera::pixel;
    std::cout << "Simulating file input from '" << context.input_file << "'" << std::endl;
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
    dStorm::engine::Image::Size sz;
    mutable dStorm::engine::Image image;
    int n;

    friend class boost::iterator_core_access;

    dStorm::engine::Image& dereference() const { return image; }
    void increment() { n++; image = dStorm::engine::Image(sz, n * cs_units::camera::frame); }
    bool equal(const _iterator& o) const { return o.n == n; }

  public:
    _iterator(int pos, dStorm::engine::Image::Size sz) 
        : sz(sz), image(sz), n(pos) { image.fill(0); }
};

Source::iterator Source::begin() {
    return iterator( _iterator(0, size) );
}
Source::iterator Source::end() {
    return iterator( _iterator(number, size) );
}

Config::Config()
: simparm::Object("DummyInput", "Dummy file input driver"),
  width("Width", "Image width", 25),
  height("Height", "Image height", 50),
  number("Number", "Number of generated images", 10)
{}

void Config::registerNamedEntries() {
    push_back( width );
    push_back( height );
    push_back( number );
}

Method::Method() 
    //"extension_dum", ".dum")
{
}

Source* Method::makeSource() {
    return new Source(config, context);
}

void Method::context_changed( ContextRef ctx )
{
    context = ctx;

    if ( static_cast<FileContext&>(*ctx).input_file != "" ) {
        MetaInfo::Ptr rv( new MetaInfo() );
        dStorm::input::Traits<dStorm::engine::Image> t;
        t.size.x() = config.width() * cs_units::camera::pixel;
        t.size.y() = config.height() * cs_units::camera::pixel;
        t.last_frame = (config.number() - 1) * cs_units::camera::frame;
        rv->traits.reset( new dStorm::input::Traits<dStorm::engine::Image>(t) );
        notify_of_trait_change( rv );
    }
}

}
