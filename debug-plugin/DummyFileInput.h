#ifndef DUMMY_FILE_INPUT_H
#define DUMMY_FILE_INPUT_H

#include <dStorm/input/Source.h>
#include <dStorm/input/Config.h>
#include <dStorm/input/FileBasedMethod.h>
#include <dStorm/engine/Image.h>
#include <dStorm/ImageTraits.h>
#include <simparm/Structure.hh>

namespace dummy_file_input {

class Method;

class Config
{
  public:
    simparm::UnsignedLongEntry width, height, number;

    Config();
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
    Source(const Method&);
    ~Source();

    iterator begin();
    iterator end();
    TraitsPtr get_traits();
};

class Method
: public Config,
  public dStorm::input::FileBasedMethod<dStorm::engine::Image>
{
  public:
    Method(dStorm::input::Config&);
    Method(const Method&, dStorm::input::Config&);

    Method* clone(dStorm::input::Config& newMaster) const
        { return new Method(*this, newMaster); }
    bool uses_input_file() const { return false; }

  protected:
    Source* impl_makeSource() { return new Source(*this); }
    void registerNamedEntries();
};

}

#endif
