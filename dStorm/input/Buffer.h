/** \file Buffer.h
 *  This file contains the definition for the Buffer class.
 */

#ifndef DSTORM_INPUT_IMAGEVECTOR_H
#define DSTORM_INPUT_IMAGEVECTOR_H

#include <memory>
#include <iterator>
#include <stdexcept>
#include <dStorm/helpers/thread.h>

#include "Config.h"
#include "Drain.h"
#include "Traits.h"
#include "Source_decl.h"
#include "Slot_decl.h"

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
     *
     *  \sa Slot
     */
    template <typename Ty> 
    class Buffer
        : public Source<Ty>, private ost::Thread
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

      protected:
        void init( std::auto_ptr< Source<Ty> > );
        /** Storage for the image Source. */
        std::auto_ptr< Source<Ty> > source;
        /** Iterators to the source */
        typename Source<Ty>::iterator current_input, end_of_input;
        /** Discarding license variable. Is set to true on WillNeverRepeatAgain message. */
        bool mayDiscard;
        bool concurrent_fetch;

        class iterator;

        /** Representation of one saved object */
        typedef std::list<Ty> Slots;

        ost::Mutex mutex;
        ost::Condition new_data;
        Slots unprocessed, current, processed;

        typename Slots::iterator get_free_slot();
        void discard( typename Slots::iterator slot );

        void run();
    };

}
}

#endif
