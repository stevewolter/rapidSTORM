#ifndef DSTORM_ENGINE_LOCALIZATIONBUNCHER_H
#define DSTORM_ENGINE_LOCALIZATIONBUNCHER_H

#include <dStorm/input/Traits.h>
#include <dStorm/Localization.h>
#include <dStorm/localization_file/reader.h>
#include <map>
#include <memory>
#include <dStorm/output/Output.h>
#include <boost/utility.hpp>
#include "Config.h"

namespace dStorm {
namespace engine_stm {

class LocalizationBuncher 
: public boost::iterator_facade< 
    LocalizationBuncher, 
    output::LocalizedImage,
    std::input_iterator_tag>
{
    class Can {
        std::list< output::Trace > traces;
        output::LocalizedImage image;

        int number_of_traces( const Localization& );
        void deep_copy(const Localization& from, 
                            data_cpp::Vector<Localization>& to);
      public:
        void push_back( const Localization& l );
        output::LocalizedImage& get() { return image; }
    };

    input::Source<LocalizationFile::Record>::iterator base;
    typedef std::map<frame_index,Can* > Canned;
    Canned canned;
    boost::shared_ptr< Can > current;
    frame_index currentImage, outputImage;

    void search_output_image();

    void print_canned_results_where_possible() throw(output::Output*);
    void can_results_or_publish( frame_index lookahead )
        throw(output::Output*);

    void put_deep_copy_into_can( const Localization &loc, Can& can );

    int last_index;
    void reset();

    friend class boost::iterator_core_access;
    output::LocalizedImage& dereference() const { return current->get(); }
    bool equal(const LocalizationBuncher& o) const;
    void increment();

  public:
    LocalizationBuncher(
        const Config&, input::Source<Localization>::iterator,
        frame_index image_number);
};

class Source
: public input::Source<output::LocalizedImage>,
  public input::Filter,
  boost::noncopyable
{
  public:
    typedef input::Source<LocalizationFile::Record> Input;
  private:
    Config config;
    std::auto_ptr< Input > base;
  public:
    Source( const Config& c, std::auto_ptr<Input> base ) 
        : config(c), base(base) {}

    iterator begin() { return LocalizationBuncher(c, base->begin()); }
    iterator end() { return LocalizationBuncher(c, base->end()); }
    TraitsPtr get_traits() {
        return TraitsPtr( new TraitsPtr::element_type(*base->get_traits()) );
    }
    BaseSource& upstream() { return *base; }
};

}
}

#endif
