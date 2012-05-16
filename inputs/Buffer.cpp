#include "debug.h"

#include "Buffer.h"

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <cassert>
#include <dStorm/engine/Image_decl.h>
#include <dStorm/engine/Image.h>
#include <dStorm/input/AdapterSource.h>
#include <dStorm/input/Method.hpp>
#include <dStorm/input/Source.h>
#include <dStorm/input/Traits.h>
#include <dStorm/Localization.h>
#include <iterator>
#include <limits>
#include <stdexcept>

namespace dStorm { 
namespace input_buffer { 

using namespace input;

template <typename Ty> 
class Source : public AdapterSource<Ty>
{
  public:
    Source(std::auto_ptr< input::Source<Ty> >);
    ~Source();

    void dispatch(BaseSource::Messages m);

    typename input::Source<Ty>::iterator begin();
    typename input::Source<Ty>::iterator end();

    BaseSource::Capabilities capabilities() const {
        return this->base().capabilities().set(BaseSource::Repeatable)
            .set(BaseSource::ConcurrentIterators);
    }
    typename input::Source<Ty>::TraitsPtr get_traits( BaseSource::Wishes );

    protected:
    void init( std::auto_ptr< Source<Ty> > );
    /** Iterators to the source, used only in non-concurrent mode */
    typename Source<Ty>::iterator current_input, end_of_input;
    /** Discarding license variable. Is set to true on WillNeverRepeatAgain message. */
    bool mayDiscard, need_to_init_iterators;
    /** When the wishes indicate no buffer is needed, this variable is set 
        *  to true to avoid buffering. */
    bool is_transparent;

    class iterator;

    /** Representation of one saved object */
    typedef std::list<Ty> Slots;

    boost::mutex mutex;
    Slots buffer;
    typename Slots::iterator next_output;

    typename Slots::iterator get_free_slot();
    void discard( typename Slots::iterator slot );
};

template<typename Type>
class Source<Type>::iterator
: public boost::iterator_facade<iterator,Type,std::forward_iterator_tag>
{
  public:
    iterator() {}
    iterator(Source& buffer);

  private:
    class referenced;
    mutable boost::shared_ptr<referenced> content;
    friend class boost::iterator_core_access;

    Type& dereference() const { return **content; }
    bool equal(const iterator& o) const 
        { return content.get() == o.content.get(); }
    void increment();

    bool isValid();
};

template<typename Type>
struct Source<Type>::iterator::referenced
{
    Source& b;
    typename Slots::iterator c;

  public:
    referenced(Source& buffer) 
        : b(buffer), c(b.get_free_slot()) {}
    boost::shared_ptr<referenced> advance() 
        { return boost::shared_ptr<referenced>(new referenced(b)); }
    ~referenced() { b.discard(c); }

    Type& operator*() { return *c; }
    const Type& operator*() const { return *c; }
    bool check() { return c != b.buffer.end(); }
};

template<typename Type>
Source<Type>::iterator::iterator(Source<Type>& buffer)
{
    content.reset( new referenced( buffer ) );
    if ( ! content->check() ) content.reset();
}

template<typename Type>
void Source<Type>::iterator::increment() 
{
    if ( content.get() != NULL ) content = content->advance();
    if ( ! content->check() ) content.reset();
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
typename Source<Object>::Slots::iterator 
Source<Object>::get_free_slot() 
{
    boost::lock_guard<boost::mutex> lock(mutex);
    while ( true ) {
        if ( next_output != buffer.end() ) {
            DEBUG("Returning stored object " << next_output->frame_number() << " for " << this);
            return next_output++;
        } else if ( current_input == end_of_input )
        {
            DEBUG("Returning empty list" << " for " << this);
            return buffer.end();
        } else {
            buffer.push_back( *current_input );
            ++current_input;
            DEBUG("Got input " << buffer.back().frame_number() << " for " << this);
            return --buffer.end();
        }
    }
}

template<typename Object>
void Source<Object>::discard( typename Slots::iterator slot ) {
    boost::lock_guard<boost::mutex> lock(mutex);
    if ( mayDiscard && slot != buffer.end() ) {
        buffer.erase( slot );
    } 
}

template<typename Object>
typename input::Source<Object>::iterator
Source<Object>::begin() 
{ 
    if ( ! is_transparent ) {
        assert( ! need_to_init_iterators );
        return typename input::Source<Object>::iterator( iterator(*this) );
    } else {
        return this->base().begin();
    }
}

template<typename Object>
typename input::Source<Object>::iterator
Source<Object>::end() 
{ 
    if ( ! is_transparent ) {
        assert( ! need_to_init_iterators );
        return typename input::Source<Object>::iterator( iterator() );
    } else {
        return this->base().end();
    }
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
    if ( !is_transparent && need_to_init_iterators ) {
        current_input = this->base().begin();
        end_of_input = this->base().end();
        need_to_init_iterators = false;
    }
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
    void attach_ui( simparm::Node& at ) { config.attach_ui( at ); }
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
