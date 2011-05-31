/** \file Buffer.h
 *  This file contains the definition for the Buffer class.
 */

#ifndef DSTORM_INPUT_IMAGEVECTOR_H
#define DSTORM_INPUT_IMAGEVECTOR_H

#include <memory>
#include <iterator>
#include <stdexcept>
#include <dStorm/helpers/thread.h>
#include <boost/thread/thread.hpp>

#include <dStorm/input/Drain.h>
#include <dStorm/input/Traits.h>
#include <dStorm/input/Source.h>
#include <dStorm/input/chain/Filter.h>

#include <dStorm/Localization_decl.h>
#include <dStorm/engine/Image_decl.h>

namespace dStorm { 
namespace input { 
    /** This class manages the image acquisition from an image
     *  source. It does so in a memory-efficient way: Only requested
     *  or delivered images are stored and images that are processed
     *  are discarded immediately.
     *
     *  Images that are marked as finished will usually be deallocated
     *  immediately. To avoid this, the discarding license may be
     *  revoked by the setDiscardingLicense() method.
     */
    template <typename Ty, bool RunConcurrently> 
    class Buffer
        : public Source<Ty>, public Filter
    {
      public:
        /** Constructor using an explicitly given input source. */
        Buffer(std::auto_ptr< Source<Ty> >);
        /** Destructor. Will deallocate all leftover images. */
        virtual ~Buffer();

        void dispatch(BaseSource::Messages m);

        simparm::Node& getConfig();
        typename Source<Ty>::TraitsPtr get_traits();

        typename Source<Ty>::iterator begin();
        typename Source<Ty>::iterator end();

        virtual BaseSource& upstream() { return *source; }

      protected:
        void init( std::auto_ptr< Source<Ty> > );
        /** Storage for the image Source. */
        std::auto_ptr< Source<Ty> > source;
        /** Iterators to the source, used only in non-concurrent mode */
        typename Source<Ty>::iterator current_input, end_of_input;
        /** Status of concurrent fetch */
        bool fetch_is_finished;
        /** Discarding license variable. Is set to true on WillNeverRepeatAgain message. */
        bool mayDiscard, need_to_init_iterators;

        class iterator;

        /** Representation of one saved object */
        typedef std::list<Ty> Slots;

        ost::Mutex mutex;
        ost::Condition new_data;
        Slots buffer;
        typename Slots::iterator next_output;
        boost::thread concurrent_fetch;

        typename Slots::iterator get_free_slot();
        void discard( typename Slots::iterator slot );

        void run();
    };

struct BufferConfig 
{
    typedef input::chain::DefaultTypes SupportedTypes;

    bool needs_concurrent_iterators, needs_multiple_passes, throw_errors;

    BufferConfig() 
        : needs_concurrent_iterators(false),
          needs_multiple_passes(false),
          throw_errors(false) {}
};

#if 0
template <class Object>
class TypedFilter
: public chain::TypedFilter<Object>,
  public virtual BufferConfig 
{
    inline chain::Link::AtEnd
        traits_changed( 
            chain::Link::TraitsRef, chain::Link*, 
            typename chain::TypedFilter<Object>::ObjectTraitsPtr );
    Source<Object>* makeSource( std::auto_ptr< Source<Object> > );
    void modify_context( input::Traits<Object>& ) {}
    void notice_context( const input::Traits<Object>& ) {}
};

template <typename Object>
Source<Object>* 
TypedFilter<Object>::makeSource( std::auto_ptr< Source<Object> > rv ) {
    if ( rv->flags.test( BaseSource::TimeCritical ) ) {
        return new Buffer<Object, true>(rv);
    } else {
        return new Buffer<Object, false>(rv);
    }
}
#endif

class BufferChainLink 
: public input::chain::Filter
{
    BufferConfig my_config;
    simparm::Object config;

    void unrecognized_traits() const;

  public:
    BufferChainLink();
    BufferChainLink* clone() const { return new BufferChainLink(*this); }

    simparm::Node& getNode() { return config; }
    AtEnd traits_changed( TraitsRef r, Link* l );
    AtEnd context_changed( ContextRef, Link* );

    BaseSource* makeSource();
};

}
}

#endif
