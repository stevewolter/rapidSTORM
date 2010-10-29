#ifndef DSTORM_ENGINE_LOCALIZATIONBUNCHER_H
#define DSTORM_ENGINE_LOCALIZATIONBUNCHER_H

#include <dStorm/input/Traits.h>
#include <dStorm/Localization.h>
#include <dStorm/localization_file/reader.h>
#include <map>
#include <memory>
#include <dStorm/output/Output.h>
#include <boost/utility.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include "Config.h"

namespace dStorm {
namespace engine_stm {

class LocalizationBuncher 
: public boost::iterator_facade< 
    LocalizationBuncher, 
    output::LocalizedImage,
    std::input_iterator_tag>
{
    typedef input::Source<LocalizationFile::Record>::iterator Base;

    enum VisitResult { KeepComing, IAmFinished };
    class Can;
    class Visitor;

    Base base, base_end;
    typedef boost::ptr_map<frame_index,Can> Canned;
    Canned canned;
    boost::shared_ptr<Can> output;
    frame_index outputImage;
    mutable output::LocalizedImage result;

    void search_output_image();

    friend class boost::iterator_core_access;
    output::LocalizedImage& dereference() const { return result; }
    bool equal(const LocalizationBuncher& o) const;
    void increment();

  public:
    LocalizationBuncher(
        const Config&, 
        Base begin, Base end,
        frame_index first_output_image);
    ~LocalizationBuncher();
};

class Source
: public input::Source<output::LocalizedImage>,
  public input::Filter,
  boost::noncopyable
{
  public:
    typedef input::Source<LocalizationFile::Record> Input;
    typedef input::Source<output::LocalizedImage> Base;
  private:
    Config config;
    std::auto_ptr< Input > base;
    frame_index firstImage, lastImage;
  public:
    Source( const Config& c, std::auto_ptr<Input> base ) 
        : Base(config, base->flags), config(c), base(base) {}

    iterator begin() 
        { return iterator( LocalizationBuncher(config, base->begin(), base->end(), firstImage) ); }
    iterator end()
        { return iterator( LocalizationBuncher(config, Input::iterator(), Input::iterator(), lastImage) ); }
    TraitsPtr get_traits();
    BaseSource& upstream() { return *base; }
};

}
}

#endif
