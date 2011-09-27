#ifndef DSTORM_ENGINE_LOCALIZATIONBUNCHER_H
#define DSTORM_ENGINE_LOCALIZATIONBUNCHER_H

#include <dStorm/input/Traits.h>
#include <dStorm/Localization.h>
#include <map>
#include <memory>
#include <dStorm/output/Output.h>
#include <boost/utility.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <dStorm/helpers/thread.h>
#include <dStorm/input/Source.h>
#include "Config.h"

namespace dStorm {
namespace engine_stm {

class Visitor;

template <typename Input>
class Source;

template <typename Input>
class LocalizationBuncher 
: public boost::iterator_facade< 
    LocalizationBuncher<Input>, 
    output::LocalizedImage,
    std::input_iterator_tag>
{
    Source<Input>& master;

    boost::shared_ptr<output::LocalizedImage> output;
    frame_index outputImage;

    void search_output_image();
    void claim_image();

    friend class boost::iterator_core_access;
    output::LocalizedImage& dereference() const { return *output; }
    bool equal(const LocalizationBuncher& o) const;
    void increment();

  public:
    LocalizationBuncher(
        const Config&, 
        Source<Input>& master,
        bool end);
    LocalizationBuncher(const LocalizationBuncher&);
    ~LocalizationBuncher();
};

template <typename InputType>
class Source
: public input::Source<output::LocalizedImage>,
  public input::Filter,
  boost::noncopyable
{
  public:
    typedef input::Source<InputType> Input;
    typedef typename Input::iterator InputIterator;
    typedef input::Source<output::LocalizedImage> Base;

  private:
    std::auto_ptr< Input > base;
  public:
    ost::Mutex mutex;
    InputIterator current, base_end;
    frame_index next_image;
    typedef boost::ptr_map<frame_index,output::LocalizedImage> Canned;
    Canned canned;

    friend class LocalizationBuncher<InputType>;

  private:
    Config config;
    frame_index firstImage, lastImage;
  public:
    Source( const Config& c, std::auto_ptr<Input> base ) ;
    ~Source();

    void dispatch(Messages m);
    iterator begin() 
        { return iterator( LocalizationBuncher<InputType>
            (config, *this, false) ); }
    iterator end()
        { return iterator( LocalizationBuncher<InputType>
            (config, *this, true) ); }
    TraitsPtr get_traits();
    BaseSource& upstream() { return *base; }
};

}
}

#endif
