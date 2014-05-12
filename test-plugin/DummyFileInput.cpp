#define BOOST_DISABLE_ASSERTS
#include "namespaces.h"
#include "test-plugin/DummyFileInput.h"
#include "input/Source.h"
#include "engine/Image.h"
#include <simparm/Entry.h>
#include "input/FileInput.h"
#include <boost/signals2/connection.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <fstream>
#include <iostream>
#include <fstream>
#include "engine/Image.h"
#include "image/constructors.h"
#include "image/MetaInfo.h"
#include "input/InputMutex.h"
#include "engine/InputTraits.h"

using namespace dStorm::input;
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
{
    simparm::Object name_object;
  public:
    simparm::Entry<unsigned long> width, height, number;
    simparm::BoolEntry goIntType;

    Config();
    void attach_ui( simparm::NodeHandle at );
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
            dStorm::image::MetaInfo<2> size_info;
            size_info.size = get_size();
            dStorm::input::Traits<dStorm::engine::ImageStack> t( size_info );
            t.image_number().range().first = 0 * boost::units::camera::frame;
            t.image_number().range().second = 
                dStorm::localization::ImageNumber::ValueType::from_value(number - 1);
            rv.reset( new dStorm::input::Traits<dStorm::engine::ImageStack>(t) );
        }
        return rv;
    }

    dStorm::image::MetaInfo<2>::Size get_size() const {
        dStorm::image::MetaInfo<2>::Size rv;
        rv.x() = width * boost::units::camera::pixel;
        rv.y() = height * boost::units::camera::pixel;
        return rv;
    }
};

class Source : public dStorm::input::Source<dStorm::engine::ImageStack>
{
    boost::mutex mutex;
    int n;
    std::auto_ptr<OpenFile> of;
    dStorm::engine::ImageStack* load();
    void dispatch(BaseSource::Messages m) { assert( !m.any() ); }
    void attach_ui_( simparm::NodeHandle ) {}
    void set_thread_count(int num_threads) OVERRIDE {}

    bool GetNext(int thread, dStorm::engine::ImageStack* output) {
        boost::lock_guard<boost::mutex> lock(mutex);
        if (n >= of->image_number()) {
            return false;
        }

        *output = dStorm::engine::ImageStack( n * boost::units::camera::frame);
        dStorm::engine::Image2D rv(of->get_size(), n * boost::units::camera::frame);
        rv.fill( 0 );
        output->push_back( rv  ); 
        ++n;
        return true;
    }

  public:
    Source(const Config&, std::auto_ptr<OpenFile> of);
    ~Source();

    TraitsPtr get_traits();
};

class Method
: public dStorm::input::FileInput<Method,OpenFile>
{
    Config config;
    simparm::BaseAttribute::ConnectionStore listening[4];

    void reread_file_locked();
    friend class dStorm::input::FileInput<Method,OpenFile>;
    OpenFile* make_file( const std::string& name ) const {
        return new OpenFile( name, config );
    }
    void modify_meta_info( dStorm::input::MetaInfo& info ) {
        info.suggested_output_basename.unformatted() = "testoutputfile";
    }
    void attach_ui( simparm::NodeHandle n ) { 
        listening[0] = config.width.value.notify_on_value_change( 
            boost::bind( &Method::reread_file_locked, this ) );
        listening[1] = config.height.value.notify_on_value_change( 
            boost::bind( &Method::reread_file_locked, this ) );
        listening[2] = config.number.value.notify_on_value_change( 
            boost::bind( &Method::reread_file_locked, this ) );
        listening[3] = config.goIntType.value.notify_on_value_change( 
            boost::bind( &Method::reread_file_locked, this ) );
        config.attach_ui(n); 
    }
    static std::string getName() { return "DummyInput"; }

  public:
    Source* makeSource();

    Method* clone() const { return new Method(*this); }

    void registerNamedEntries();
};

Source::Source(const Config& config, std::auto_ptr<OpenFile> of) 
: of( of )
{
    assert( of.get() );
}

Source::~Source() {}

Source::TraitsPtr
Source::get_traits()
{
    assert( of.get() );
    return TraitsPtr( dynamic_cast< Traits* >(of->getTraits().release()) );
}

Config::Config()
: name_object("DummyInput", "Dummy file input driver"),
  width("Width", "Image width", 25),
  height("Height", "Image height", 50),
  number("Number", "Number of generated images", 10),
  goIntType("GoIntType", "Make input source of type int", false)
{
    name_object.set_user_level( simparm::Debug );
}

void Config::attach_ui( simparm::NodeHandle n) {
    simparm::NodeHandle at = name_object.attach_ui(n);
    width.attach_ui( at );
    height.attach_ui( at );
    number.attach_ui( at );
    goIntType.attach_ui( at );
}

Source* Method::makeSource() {
    return new Source(config, get_file());
}

void Method::reread_file_locked() {
    dStorm::input::InputMutexGuard lock( global_mutex() );
    reread_file();
}

std::auto_ptr< dStorm::input::Link >
    make()
{
    return std::auto_ptr< dStorm::input::Link >( new Method() );
}

}
