#include "DummyFileInput.h"
#include <iostream>
#include <dStorm/engine/Image_impl.h>

using namespace dStorm::input;

namespace dummy_file_input {

Source::Source(const Method& config) 
: simparm::Set("DummyInput", "Dummy input"),
  dStorm::input::SerialSource<dStorm::engine::Image>
    ( static_cast<simparm::Node&>(*this), BaseSource::Pushing | BaseSource::Pullable),
  w( config.width() ),
  h( config.height() ),
  number( config.number() )
{
    std::cout << "Simulating file input from '" << config.inputFile().c_str() << "'" << std::endl;
    dStorm::input::Traits<dStorm::engine::Image>& traits = *this;
    traits.size.x() = config.width()* cs_units::camera::pixel;
    traits.size.y() = config.height()* cs_units::camera::pixel;
}

Source::~Source() {}

dStorm::engine::Image* Source::load() {
    return new dStorm::engine::Image( w, h );
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
