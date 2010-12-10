/** \file Buffer.h
 *  This file contains the declaration for the Buffer class.
 */

#ifndef CImgBuffer_IMAGEVECTOR_H
#define CImgBuffer_IMAGEVECTOR_H

#include <CImgBuffer/Config.h>
#include <CImgBuffer/Drain.h>
#include <memory>
#include <stdexcept>
#include <data-c++/Vector.h>

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
    template <typename T> class Buffer 
        : public Drain<T>,
          public data_cpp::Vector< Slot<T> >
    {
      public:
        /** Constructor using the input method defined in the Config's
         *  \c inputMethod field. The right input source will auto-
         *  magically be built. */
        Buffer(const Config<T> &) throw(std::exception);
        /** Constructor using an explicitly given input source. */
        Buffer(std::auto_ptr< Source<T> >) throw(std::exception);
        /** Destructor. Will deallocate all leftover images. */
        virtual ~Buffer() throw();

        /** Returns the width common to all images in the Buffer. */
        int dimx() const throw() ;
        /** Returns the height common to all images in the Buffer. */
        int dimy() const throw() ;
        /** Returns the depth common to all images in the Buffer. */
        int dimz() const throw() ;
        /** Returns the color range common to all images in the Buffer. */
        int dimv() const throw() ;

        /** Reset all images to the Untouched state. */
        void makeAllUntouched() throw(); 
        /** Set the discarding license. The discarding license defines
         *  whether images whose state is set to Finished will be
         *  discarded; if it is set to the default of \c true, they
         *  will be deleted. When the discarding license is set to true
         *  and was false, all images that are finished will be deleted
         *  immediately. */
        void setDiscardingLicense(bool mayDiscard) throw();

        /** Insert a single image into the Buffer. */
        void accept(int index, std::auto_ptr< T > i) throw();

        ConfigNode& getConfig() throw();
        const Source<T>& getSource() const throw() { return *source; }

      private:
        /** Storage for the image Source. */
        std::auto_ptr< Source<T> > source;
        /** Discarding license variable. See setDiscardingLicense() */
        bool mayDiscard;

        /** Does the initializating one \c source is set. */
        void init(int fi, int li) throw(std::exception);
    };
}

#endif
