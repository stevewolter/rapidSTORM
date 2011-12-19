/** \file Buffer.h
 *  This file contains the definition for the Buffer class.
 */

#ifndef DSTORM_INPUT_IMAGEVECTOR_H
#define DSTORM_INPUT_IMAGEVECTOR_H

#include <dStorm/stack_realign.h>
#include <memory>
#include <iterator>
#include <stdexcept>
#include <dStorm/helpers/thread.h>
#include <boost/thread/thread.hpp>

#include <dStorm/input/Drain.h>
#include <dStorm/input/Traits.h>
#include <dStorm/input/AdapterSource.h>
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
    class Buffer : public AdapterSource<Ty>
    {
      public:
        /** Constructor using an explicitly given input source. */
        Buffer(std::auto_ptr< Source<Ty> >);
        /** Destructor. Will deallocate all leftover images. */
        virtual ~Buffer();

        void dispatch(BaseSource::Messages m);

        typename Source<Ty>::iterator begin();
        typename Source<Ty>::iterator end();

        BaseSource::Capabilities capabilities() const {
            return this->base().capabilities().set(BaseSource::Repeatable)
                .set(BaseSource::ConcurrentIterators);
        }
        typename Source<Ty>::TraitsPtr get_traits( BaseSource::Wishes );

      protected:
        void init( std::auto_ptr< Source<Ty> > );
        /** Iterators to the source, used only in non-concurrent mode */
        typename Source<Ty>::iterator current_input, end_of_input;
        /** Status of concurrent fetch */
        bool fetch_is_finished;
        /** Discarding license variable. Is set to true on WillNeverRepeatAgain message. */
        bool mayDiscard, need_to_init_iterators;
        /** When the wishes indicate no buffer is needed, this variable is set 
         *  to true to avoid buffering. */
        bool is_transparent;

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

        DSTORM_REALIGN_STACK void run();
    };

}
}

#endif
