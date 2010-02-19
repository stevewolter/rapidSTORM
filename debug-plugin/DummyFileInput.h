#ifndef DUMMY_FILE_INPUT_H
#define DUMMY_FILE_INPUT_H

#include <dStorm/input/SerialSource.h>
#include <dStorm/input/Config.h>
#include <dStorm/input/FileBasedMethod.h>
#include <dStorm/engine/Image.h>
#include <dStorm/input/ImageTraits.h>
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
               public dStorm::input::SerialSource<dStorm::engine::Image>
{
    int w, h, number;
  public:
    Source(const Method&);
    ~Source();

    int quantity() const { return number; }
    dStorm::engine::Image* load();
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

  protected:
    Source* impl_makeSource() { return new Source(*this); }
    void registerNamedEntries();
};

}

#endif
