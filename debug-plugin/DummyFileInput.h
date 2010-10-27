#ifndef DUMMY_FILE_INPUT_H
#define DUMMY_FILE_INPUT_H

#include <dStorm/input/Source_impl.h>
#include <dStorm/input/Config.h>
#include <dStorm/engine/Image.h>
#include <dStorm/ImageTraits.h>
#include <simparm/Structure.hh>
#include <simparm/NumericEntry.hh>
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/Link.h>

namespace dummy_file_input {

class Method;

class Config
: public simparm::Object
{
  public:
    simparm::UnsignedLongEntry width, height, number;

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
  public:
    Source(const Config&, dStorm::input::chain::Context::Ptr);
    ~Source();

    iterator begin();
    iterator end();
    TraitsPtr get_traits();
};

class Method
: public dStorm::input::chain::Terminus
{
    simparm::Structure<Config> config;
    dStorm::input::chain::Context::Ptr context;

  public:
    Method();

    virtual void context_changed( ContextRef, Link* );

    virtual Source* makeSource();
    virtual simparm::Node& getNode() { return config; }

    Method* clone() const { return new Method(*this); }

};

}

#endif
