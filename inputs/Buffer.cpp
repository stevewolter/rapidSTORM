#include "debug.h"

#include "Buffer.h"

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <cassert>
#include <dStorm/engine/Image_decl.h>
#include <dStorm/engine/Image.h>
#include <dStorm/input/AdapterSource.h>
#include <dStorm/input/Method.hpp>
#include <dStorm/input/Source.h>
#include <dStorm/input/Traits.h>
#include <dStorm/Localization.h>
#include <limits>
#include <stdexcept>

namespace dStorm { 
namespace input_buffer { 

using namespace input;

template <typename Ty> 
class Source : public AdapterSource<Ty>
{
    void attach_local_ui_( simparm::NodeHandle ) {}
    bool GetNext(int thread, Ty* target) override;
  public:
    Source(std::auto_ptr< input::Source<Ty> >);
    ~Source();

    void dispatch(BaseSource::Messages m);

    BaseSource::Capabilities capabilities() const {
        return this->base().capabilities().set(BaseSource::Repeatable)
            .set(BaseSource::ConcurrentIterators);
    }
    typename input::Source<Ty>::TraitsPtr get_traits( BaseSource::Wishes );

  protected:
    void init( std::auto_ptr< Source<Ty> > );
    /** Discarding license variable. Is set to true on WillNeverRepeatAgain message. */
    bool mayDiscard, need_to_init_iterators;
    /** When the wishes indicate no buffer is needed, this variable is set 
        *  to true to avoid buffering. */
    bool is_transparent;

    /** Representation of one saved object */
    typedef std::list<Ty> Slots;

    boost::mutex mutex;
    Slots buffer;
    typename Slots::iterator next_output;

    void discard( typename Slots::iterator slot );
    void set_thread_count(int num_threads) override {
        if (is_transparent) {
            AdapterSource<Ty>::set_thread_count(num_threads);
        } else {
            AdapterSource<Ty>::set_thread_count(1);
        }
    }
};

template<typename Type>
bool Source<Type>::GetNext(int thread, Type* target) {
    if (is_transparent) {
        return AdapterSource<Type>::GetNext(thread, target);
    }

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
  is_transparent(true),
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
Source<Object>::get_traits( BaseSource::Wishes w ) 
{
    typename Source<Object>::TraitsPtr t = this->base().get_traits(w);
    BaseSource::Capabilities c = this->base().capabilities();
    BaseSource::Capability providing[] = 
        { BaseSource::Repeatable, BaseSource::ConcurrentIterators };
    is_transparent = true;
    for (int i = 0; i < 2; ++i)
        is_transparent = is_transparent &&
            ( c.test( providing[i] ) || ! w.test( providing[i] ) );
    return t;
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
