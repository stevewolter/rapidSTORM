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

class Source : public simparm::Set,
               public dStorm::input::Source<dStorm::engine::Image>
{
    dStorm::engine::Image::Size size;
    int number;
    dStorm::engine::Image* load();
    class _iterator;
    typedef dStorm::input::Source<dStorm::engine::Image>::iterator iterator;
    void dispatch(BaseSource::Messages m) { assert( !m.any() ); }
  public:
    Source(const Config&, dStorm::input::chain::Context::ConstPtr);
    ~Source();

    iterator begin();
    iterator end();
    TraitsPtr get_traits();
};

class Method
: public dStorm::input::FileInput,
  simparm::Listener
{
    simparm::Structure<Config> config;
    dStorm::input::chain::Context::ConstPtr context;
    std::string currently_loaded_file;

    void operator()(const simparm::Event&);
    AtEnd make_new_traits();

  public:
    Method();
    Method(const Method&);

    AtEnd context_changed( ContextRef, Link* );

    Source* makeSource();
    simparm::Node& getNode() { return config; }

    Method* clone() const { return new Method(*this); }

    void registerNamedEntries();
};

}

#endif
