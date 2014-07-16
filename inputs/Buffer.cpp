#include "debug.h"
#include "inputs/Buffer.h"

#include <cassert>
#include <limits>
#include <list>
#include <stdexcept>

#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

#include "engine/Image_decl.h"
#include "engine/Image.h"
#include "helpers/make_unique.hpp"
#include "input/AdapterSource.h"
#include "input/FilterFactory.h"
#include "input/Source.h"
#include "input/Traits.h"
#include "Localization.h"

namespace dStorm { 
namespace input_buffer { 

using namespace input;

class Source : public AdapterSource<engine::ImageStack> {
    void attach_local_ui_( simparm::NodeHandle ) {}
    bool GetNext(int thread, engine::ImageStack* target) OVERRIDE;
  public:
    Source(std::unique_ptr< input::Source<engine::ImageStack> >);

    void dispatch(BaseSource::Messages m);

    typename input::Source<engine::ImageStack>::TraitsPtr get_traits();

  protected:
    /** Discarding license variable. Is set to true on WillNeverRepeatAgain message. */
    bool mayDiscard, need_to_init_iterators;

    /** Representation of one saved object */
    typedef std::list<engine::ImageStack> Slots;

    boost::mutex mutex;
    Slots buffer;
    typename Slots::iterator next_output;

    void discard( typename Slots::iterator slot );
    void set_thread_count(int num_threads) OVERRIDE {
        AdapterSource<engine::ImageStack>::set_thread_count(1);
    }
};

bool Source::GetNext(int thread, engine::ImageStack* target) {
    boost::lock_guard<boost::mutex> lock(mutex);
    if ( next_output != buffer.end() ) {
        DEBUG("Returning stored object " << next_output->frame_number() << " for " << this);
        *target = *next_output;
        if (mayDiscard) {
            next_output = buffer.erase(next_output);
        } else {
            ++next_output;
        }
        return true;
    } else {
        if (!AdapterSource<engine::ImageStack>::GetNext(0, target)) {
            return false;
        }

        if (!mayDiscard) {
            buffer.push_back(*target);
        }
        return true;
    }
}

Source::Source(std::unique_ptr< input::Source<engine::ImageStack> > src) 
: AdapterSource<engine::ImageStack>(std::move(src)),
  mayDiscard( false ), need_to_init_iterators(false),
  next_output( buffer.begin() ) {
    need_to_init_iterators = true;
}

void Source::dispatch(BaseSource::Messages m) {
    DEBUG("Dispatching message " << m.to_string() << " to buffer");
    if ( m.test( BaseSource::WillNeverRepeatAgain ) ) {
        m.reset( BaseSource::WillNeverRepeatAgain );
        boost::lock_guard<boost::mutex> lock(mutex);
        if ( !mayDiscard ) {
            mayDiscard = true;
            buffer.erase( buffer.begin(), next_output );
        }
    }
    if ( m.test( BaseSource::RepeatInput ) ) {
        m.reset( BaseSource::RepeatInput );
        boost::lock_guard<boost::mutex> lock(mutex);
        if ( mayDiscard ) throw std::runtime_error("Buffer is not repeatable any more");
        next_output = buffer.begin();
    }
    this->base().dispatch(m);
    DEBUG("Dispatched message " << m.to_string() << " to buffer " << this);
}

typename input::Source<engine::ImageStack>::TraitsPtr
Source::get_traits() 
{
    return this->base().get_traits();
}

class Factory : public input::FilterFactory<engine::ImageStack> {
  public:
    Factory() : config("Buffer", "Buffer") {}
    Factory* clone() const OVERRIDE { return new Factory(*this); }
    void attach_ui(simparm::NodeHandle at,
                   std::function<void()> traits_change_callback) OVERRIDE {
        config.attach_ui( at );
    }
    std::unique_ptr<input::Source<engine::ImageStack>> make_source(
        std::unique_ptr<input::Source<engine::ImageStack>> input) OVERRIDE {
        return make_unique<Source>(std::move(input));
    }
    boost::shared_ptr<const input::Traits<engine::ImageStack>> make_meta_info(
        boost::shared_ptr<const input::Traits<engine::ImageStack>> input_meta_info) OVERRIDE {
        return input_meta_info;
    }

  private:
    simparm::Object config;
};

std::unique_ptr<input::FilterFactory<engine::ImageStack>> create() {
    return make_unique<Factory>();
}

}
}
