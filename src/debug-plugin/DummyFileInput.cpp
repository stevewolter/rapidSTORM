#include "DummyFileInput.h"
#include <iostream>
#include <dStorm/engine/Image.h>

using namespace dStorm::input;

namespace dummy_file_input {

Source::Source(const Method& config) 
: simparm::Set("YDummyInput", "Dummy input"),
  dStorm::input::Source<dStorm::engine::Image>
    ( static_cast<simparm::Node&>(*this), Capabilities() ),
  w( config.width() ),
  h( config.height() ),
  number( config.number() )
{
    std::cout << "Simulating file input from '" << config.inputFile().c_str() << "'" << std::endl;
}

Source::~Source() {}

Source::TraitsPtr
Source::get_traits()
{
    TraitsPtr rv( new TraitsPtr::element_type() );
    rv->size.x() = w* cs_units::camera::pixel;
    rv->size.y() = h* cs_units::camera::pixel;
    rv->last_frame = (number - 1) * cs_units::camera::frame;
}

dStorm::engine::Image* Source::load() {
    dStorm::engine::Image::Size sz;
    sz.x() = w * cs_units::camera::pixel;
    sz.y() = h * cs_units::camera::pixel;
    return new dStorm::engine::Image( sz );
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
