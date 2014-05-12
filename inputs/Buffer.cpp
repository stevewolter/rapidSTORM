#include "debug.h"

#include "inputs/Buffer.h"

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <cassert>
#include "engine/Image_decl.h"
#include "engine/Image.h"
#include "input/AdapterSource.h"
#include "input/Method.hpp"
#include "input/Source.h"
#include "input/Traits.h"
#include "Localization.h"
#include <limits>
#include <stdexcept>

namespace dStorm { 
namespace input_buffer { 

using namespace input;

template <typename Ty> 
class Source : public AdapterSource<Ty>
{
    void attach_local_ui_( simparm::NodeHandle ) {}
    bool GetNext(int thread, Ty* target) OVERRIDE;
  public:
    Source(std::auto_ptr< input::Source<Ty> >);
    ~Source();

    void dispatch(BaseSource::Messages m);

    typename input::Source<Ty>::TraitsPtr get_traits();

  protected:
    void init( std::auto_ptr< Source<Ty> > );
    /** Discarding license variable. Is set to true on WillNeverRepeatAgain message. */
    bool mayDiscard, need_to_init_iterators;

    /** Representation of one saved object */
    typedef std::list<Ty> Slots;

    boost::mutex mutex;
    Slots buffer;
    typename Slots::iterator next_output;

    void discard( typename Slots::iterator slot );
    void set_thread_count(int num_threads) OVERRIDE {
        AdapterSource<Ty>::set_thread_count(1);
    }
};

template<typename Type>
bool Source<Type>::GetNext(int thread, Type* target) {
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
        if (!AdapterSource<Type>::GetNext(0, target)) {
            return false;
        }

        if (!mayDiscard) {
            buffer.push_back(*target);
        }
        return true;
    }
}

template <typename Object>
Source<Object>::Source(std::auto_ptr< input::Source<Object> > src) 
: AdapterSource<Object>(src),
  mayDiscard( false ), need_to_init_iterators(false),
  next_output( buffer.begin() )
{
    need_to_init_iterators = true;
}

template<typename Object>
Source<Object>::~Source() {
}

template<typename Object>
void Source<Object>::dispatch(BaseSource::Messages m) {
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

template<typename Object>
typename input::Source<Object>::TraitsPtr
Source<Object>::get_traits() 
{
    return this->base().get_traits();
}

class ChainLink 
: public input::Method<ChainLink>
{
    simparm::Object config;

    friend class input::Method<ChainLink>;
    template <typename Type>
    BaseSource* make_source( std::auto_ptr< input::Source<Type> > p ) 
        { return new Source<Type>(p); }
    template <typename Type>
    void update_traits( MetaInfo&, Traits<Type>& ) {}
    template <typename Type>
    bool changes_traits( const MetaInfo&, const Traits<Type>& )
        { return false; }

  public:
    ChainLink();
    static std::string getName() { return "Buffer"; }
    void attach_ui( simparm::NodeHandle at ) { config.attach_ui( at ); }
};

ChainLink::ChainLink() 
: config(getName(), "Buffer") 
{
}

std::auto_ptr<Link> makeLink() { 
    return std::auto_ptr<Link>( new ChainLink() );
}

}
}
