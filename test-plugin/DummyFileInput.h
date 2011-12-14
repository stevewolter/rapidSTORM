#ifndef DUMMY_FILE_INPUT_H
#define DUMMY_FILE_INPUT_H

#include <dStorm/input/Source_impl.h>
#include <dStorm/input/Config.h>
#include <dStorm/engine/Image.h>
#include <dStorm/ImageTraits.h>
#include <simparm/Structure.hh>
#include <simparm/Entry.hh>
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/FileInput.h>
#include <boost/signals2/connection.hpp>
#include <fstream>

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
        if ( std::ifstream(filename.c_str(), std::ios_base::in) )
            throw std::runtime_error(filename + " does not exist");
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
  public:
    Source(const Config&, boost::shared_ptr<OpenFile> of);
    ~Source();

    iterator begin();
    iterator end();
    TraitsPtr get_traits();
};

class Method
: public dStorm::input::FileInput<Method,OpenFile>,
  simparm::Listener
{
    simparm::Structure<Config> config;

    void operator()(const simparm::Event&);
    friend class dStorm::input::FileInput<Method,OpenFile>;
    OpenFile* make_file( const std::string& name ) {
        return new OpenFile( name, config );
    }
    void modify_meta_info( dStorm::input::chain::MetaInfo& info ) {
        info.suggested_output_basename.unformatted() = "testoutputfile";
    }

  public:
    Method();

    Source* makeSource();
    simparm::Node& getNode() { return config; }

    Method* clone() const { return new Method(*this); }

    void registerNamedEntries();
};

}

#endif
