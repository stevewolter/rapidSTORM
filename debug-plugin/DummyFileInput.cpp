#include "debug.h"
#include "DummyFileInput.h"
#include <iostream>
#include <fstream>
#include <dStorm/engine/Image.h>
#include <boost/iterator/iterator_facade.hpp>
#include <dStorm/input/chain/FileContext.h>

using namespace dStorm::input;
using namespace dStorm::input::chain;

namespace dStorm {
namespace input {

template <>
class Traits<int> 
: public BaseTraits {
    Traits<int>* clone() const { return new Traits<int>(*this); }
    std::string desc() const { return "int"; }
};

}
}

namespace dummy_file_input {

Source::Source(const Config& config, Context::ConstPtr ptr) 
: simparm::Set("YDummyInput", "Dummy input"),
  dStorm::input::Source<dStorm::engine::Image>
    ( static_cast<simparm::Node&>(*this), Capabilities() ),
  number( config.number() )
{
    const FileContext& context = static_cast<const FileContext&>(*ptr);
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

Method::Method(const Method& o) 
: dStorm::input::chain::Terminus(o),
  simparm::Listener( simparm::Event::ValueChanged ),
  config(o.config),
  context(o.context),
  currently_loaded_file(o.currently_loaded_file)
{
    registerNamedEntries();
}

Source* Method::makeSource() {
    return new Source(config, context);
}

Method::AtEnd Method::make_new_traits() {
    MetaInfo::Ptr rv( new dStorm::input::chain::FileMetaInfo() );
    if ( config.goIntType() ) {
        DEBUG("Publishing int traits");
        rv->set_traits( new dStorm::input::Traits<int>() );
    } else {
        dStorm::input::Traits<dStorm::engine::Image> t;
        t.size.x() = config.width() * cs_units::camera::pixel;
        t.size.y() = config.height() * cs_units::camera::pixel;
        t.last_frame = (config.number() - 1) * cs_units::camera::frame;
        rv->set_traits( new dStorm::input::Traits<dStorm::engine::Image>(t) );
    }
    rv->suggested_output_basename.unformatted() = "testoutputfile";
    return notify_of_trait_change( rv );
}

Method::AtEnd Method::context_changed( ContextRef ctx, Link* l )
{
    Terminus::context_changed(ctx, l);
    context = ctx;

    std::string new_file = static_cast<const FileContext&>(*ctx).input_file;
    if ( new_file != currently_loaded_file ) {
        if ( new_file != "" ) {
            std::ifstream test(new_file.c_str(), std::ios_base::in);
            if ( test ) {
                DEBUG("Successfully opened file " << new_file );
                currently_loaded_file = new_file;
                return make_new_traits();
            } else {
                DEBUG("Could not open file " << new_file );
                return notify_of_trait_change( MetaInfo::Ptr() );
            }
        } else {
            DEBUG("Publishing empty traits for empty file name");
            currently_loaded_file = new_file;
            return notify_of_trait_change( MetaInfo::Ptr() );
        }
    } else {
        DEBUG("Publishing no traits since input file did not change");
        return Method::AtEnd();
    }
}

void Method::operator()(const simparm::Event& e) {
    make_new_traits();
}

void Method::registerNamedEntries() {
    receive_changes_from( config.width.value );
    receive_changes_from( config.height.value );
    receive_changes_from( config.number.value );
    receive_changes_from( config.goIntType.value );
}

}
