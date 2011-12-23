#define BOOST_DISABLE_ASSERTS
#include <dStorm/namespaces.h>
#include <dStorm/log.h>
#include "DummyFileInput.h"
#include <dStorm/input/Source.h>
#include <dStorm/engine/Image.h>
#include <dStorm/ImageTraits.h>
#include <simparm/Structure.hh>
#include <simparm/Entry.hh>
#include <dStorm/input/FileInput.h>
#include <boost/signals2/connection.hpp>
#include <fstream>
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

class Method;

class Config
: public simparm::Object
{
  public:
    simparm::Entry<unsigned long> width, height, number;
    simparm::BoolEntry goIntType;

    Config();
    void registerNamedEntries();
};

class OpenFile 
{
    int width, height, number;
    bool int_type;
  public:
    OpenFile( std::string filename, const Config& c ) 
        : width(c.width()), height(c.height()), number(c.number()) ,
          int_type( c.goIntType() )
    {
        if ( filename.size() < 6 || filename.substr( filename.size() - 6 ) != ".dummy" )    
            throw std::runtime_error("Filenames for dummy input must end in .dummy");
    }

    int image_number() const { return number; }

    std::auto_ptr< dStorm::input::BaseTraits > getTraits() {
        std::auto_ptr< dStorm::input::BaseTraits > rv;
        if ( int_type ) {
            rv.reset( new dStorm::input::Traits<int>() );
        } else {
            dStorm::input::Traits<dStorm::engine::Image> t;
            t.size = get_size();
            t.image_number().range().first = 0 * boost::units::camera::frame;
            t.image_number().range().second = 
                dStorm::traits::ImageNumber::ValueType::from_value(number - 1);
            rv.reset( new dStorm::input::Traits<dStorm::engine::Image>(t) );
        }
        return rv;
    }

    dStorm::engine::Image::Size get_size() const {
        dStorm::engine::Image::Size rv;
        rv.fill( 1 * boost::units::camera::pixel );
        rv.x() = width * boost::units::camera::pixel;
        rv.y() = height * boost::units::camera::pixel;
        return rv;
    }
};

class Source : public simparm::Set,
               public dStorm::input::Source<dStorm::engine::Image>
{
    boost::shared_ptr<OpenFile> of;
    dStorm::engine::Image* load();
    class _iterator;
    typedef dStorm::input::Source<dStorm::engine::Image>::iterator iterator;
    void dispatch(BaseSource::Messages m) { assert( !m.any() ); }
    simparm::Node& node() { return *this; }
  public:
    Source(const Config&, boost::shared_ptr<OpenFile> of);
    ~Source();

    iterator begin();
    iterator end();
    TraitsPtr get_traits( Wishes );
    Capabilities capabilities() const { return Capabilities(); }
};

class Method
: public dStorm::input::FileInput<Method,OpenFile>,
  simparm::Listener
{
    simparm::Structure<Config> config;

    void operator()(const simparm::Event&);
    friend class dStorm::input::FileInput<Method,OpenFile>;
    OpenFile* make_file( const std::string& name ) const {
        return new OpenFile( name, config );
    }
    void modify_meta_info( dStorm::input::chain::MetaInfo& info ) {
        info.suggested_output_basename.unformatted() = "testoutputfile";
    }
    simparm::Object& getNode() { return config; }

  public:
    Method();

    Source* makeSource();

    Method* clone() const { return new Method(*this); }

    void registerNamedEntries();
};

Source::Source(const Config& config, boost::shared_ptr<OpenFile> of) 
: simparm::Set("YDummyInput", "Dummy input"),
  of( of )
{
    assert( of.get() );
}

Source::~Source() {}

Source::TraitsPtr
Source::get_traits( Wishes )
{
    assert( of.get() );
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

std::auto_ptr< dStorm::input::chain::Link >
    make()
{
    return std::auto_ptr< dStorm::input::chain::Link >( new Method() );
}

}
