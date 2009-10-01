/** \file Buffer.h
 *  This file contains the declaration for the Buffer class.
 */

#ifndef CImgBuffer_IMAGEVECTOR_H
#define CImgBuffer_IMAGEVECTOR_H

#include <CImgBuffer/Config.h>
#include <CImgBuffer/Drain.h>
#include <CImgBuffer/Traits.h>
#include <memory>
#include <stdexcept>
#include <data-c++/Vector.h>
#include <cc++/thread.h>

namespace CImgBuffer { 
    template <typename T> class Source;
    template <typename T> class Slot;

    /** This class manages the image acquisition from an image
     *  source. It does so in a memory-efficient way: Only requested
     *  or delivered images are stored and images that are processed
     *  are discarded immediately.
     *
     *  The Buffer class separates the images returned by an
     *  ImageSource into different Slot objects, addressed by
     *  the image number. This allows implementing classes to
     *  address the resulting images by index number and attach
     *  a state to them; see the Slot class for details.
     *
     *  The public interface of this class is basically that
     *  of a vector. All images contained in the Slot objects
     *  are guaranteed to have the same dimensions, which can
     *  be accessed with the dimx(), dimy(), dimz() and dimv()
     *  methods. Images are inserted into the buffer with the
     *  accept() method; this is usually left to the image sources.
     *
     *  Images that are marked as finished will usually be deallocated
     *  immediately. To avoid this, the discarding license may be
     *  revoked by the setDiscardingLicense() method.
     *
     *  \sa Slot
     */
    template <typename Ty> class Buffer 
        : public Drain<Ty>,
          private data_cpp::Vector< Slot<Ty> >
    {
      public:
        /** Constructor using the input method defined in the Config's
         *  \c inputMethod field. The right input source will auto-
         *  magically be built. */
        Buffer(const Config &);
        /** Constructor using an explicitly given input source. */
        Buffer(std::auto_ptr< Source<Ty> >);
        /** Destructor. Will deallocate all leftover images. */
        virtual ~Buffer();

        /** Reset all images to the Untouched state. */
        void makeAllUntouched(); 
        /** Set the discarding license. The discarding license defines
         *  whether images whose state is set to Finished will be
         *  discarded; if it is set to the default of \c true, they
         *  will be deleted. When the discarding license is set to true
         *  and was false, all images that are finished will be deleted
         *  immediately. */
        void setDiscardingLicense(bool mayDiscard);

        /** Insert a single image into the Buffer. */
        Management
            accept(int index, int number, Ty* i);

        simparm::Node& getConfig();
        const Source<Ty>& getSource() const { return *source; }

        const Traits<Ty>& getTraits() const { return *source; }

        void receive_number_of_objects(int);

        typedef typename data_cpp::Vector< Slot<Ty> >::iterator iterator;
        typedef typename data_cpp::Vector< Slot<Ty> >::const_iterator
            const_iterator;

        typename data_cpp::Vector< Slot<Ty> >::iterator begin() 
            { await_init(); return data_cpp::Vector< Slot<Ty> >::begin(); }
        typename data_cpp::Vector< Slot<Ty> >::const_iterator begin() const
            { await_init(); return data_cpp::Vector< Slot<Ty> >::begin(); }
        typename data_cpp::Vector< Slot<Ty> >::iterator end() 
            { await_init(); return data_cpp::Vector< Slot<Ty> >::end(); }
        typename data_cpp::Vector< Slot<Ty> >::const_iterator end() const
            { await_init(); return data_cpp::Vector< Slot<Ty> >::end(); }

        size_t size() const
            { await_init(); return data_cpp::Vector< Slot<Ty> >::size(); }

        void signal_end_of_acquisition();

      protected:
        /** Storage for the image Source. */
        std::auto_ptr< Source<Ty> > source;
        /** Discarding license variable. See setDiscardingLicense() */
        bool mayDiscard;

        /** Mutex protects the initialized condition. */
        ost::Mutex initialization;
        /** Condition for waiting for initialization. */
        ost::Condition finished_initialization;
        /** Variable set to true when initialization finished. */
        bool initialized;

        /** Method used to block until initialization. */
        void await_init() const { 
            if (initialized) return;
            ost::MutexLock lock(const_cast<ost::Mutex&>(initialization));
            while (!initialized) {
                const_cast<ost::Condition&>(finished_initialization).wait();
            }
        }

        /** Does the initializating one \c source is set. */
        void init();
    };
}

#endif
