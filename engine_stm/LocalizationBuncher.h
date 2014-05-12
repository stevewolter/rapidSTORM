#ifndef DSTORM_ENGINE_LOCALIZATIONBUNCHER_H
#define DSTORM_ENGINE_LOCALIZATIONBUNCHER_H

#include "input/Traits.h"
#include "Localization.h"
#include <map>
#include <memory>
#include "output/Output.h"
#include <boost/utility.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/thread/mutex.hpp>
#include "input/Source.h"

namespace dStorm {
namespace engine_stm {

class Visitor;

template <typename Input>
class Source;

template <typename InputType>
class Source
: public input::Source<output::LocalizedImage>,
  boost::noncopyable
{
  public:
    typedef input::Source<InputType> Input;
    typedef input::Source<output::LocalizedImage> Base;

  private:
    std::auto_ptr< Input > base;
    frame_index first_image, current_image;
    boost::optional<frame_index> last_image;
    std::map<frame_index, output::LocalizedImage> canned;
    bool in_sequence;
    bool input_left_over, input_exhausted;
    InputType input;

    void attach_ui_( simparm::NodeHandle n ) { base->attach_ui(n); }
    bool GetNext(int thread, output::LocalizedImage* target) OVERRIDE;
    void set_thread_count(int threads) OVERRIDE;

    void ReadImage(output::LocalizedImage* target);
    void CollectEntireImage(output::LocalizedImage* target);

  public:
    std::auto_ptr<output::LocalizedImage> read( frame_index );
    bool is_finished( frame_index current ) const;

  public:
    Source( std::auto_ptr<Input> base ) ;
    ~Source();

    void dispatch(Messages m);
    TraitsPtr get_traits();
    BaseSource& upstream() { return *base; }
};

}
}

#endif
