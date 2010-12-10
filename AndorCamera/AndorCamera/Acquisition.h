/** \file 
 *  This file contains the AndorCamera::Acquisition class declaration. */
#ifndef ANDORCAMERA_ACQUISITION_H
#define ANDORCAMERA_ACQUISITION_H

#include <AndorCamera/ControlReference.h>
#include <stdint.h>
#include <time.h>
#include <cc++/thread.h>

namespace AndorCamera {
    /** This class is the public interface
     *  for acquiring images from an Andor camera. 
     *
     *  After construction, the Acquisition will be in a
     *  waiting state. No images will be acquired, and the
     *  camera will not be initialized or cooled.
     *  Once the start() function is called, successive calls to 
     *  getNextImage() will fill the provided buffer with
     *  the acquired images.
     *
     *  An acquisition may be stopped by calling its stop() method.
     *  This method will either be called by the API user or by
     *  the Control class; in the latter case, getNextImage() will
     *  unexpectedly return -1.
     *
     *  To save memory, the Acquisition class internally divides
     *  its operation into runs of \c runSize images.
     *
     *  Acquisition provides a public mutex interface to allow atomic
     *  operations. As long as an acquisition is locked, it can not be
     *  swapped from the camera by another acquisition.
     *  */
    class Acquisition : public ost::Mutex {
      private:
        ControlReference control;
        int l /** Left edge of acquisition area */,
            r /** Right edge of acquisition area */,
            t /** Top  edge of acquisition area */,
            b /** Bottom edge of acquisition area */;
        /** The next image in the acquisition sequence */
        unsigned long nextImage; 
        /** The next image in the run sequence */
        int imageInRun; 
        /** The total number of images this acquisition should acquire. */
        unsigned long numImages;
        /** The total number of images this run should acquire. */
        int runSize;
        /** The last image that is present in the Andor SDK ring buffer.*/
        int lastValidImage;
        /** True if this acquisition was set to be the current
         *  acquisition in the Control. */
        bool isCurrentAcquisition;
        /** True if this acquisition is currently running.
         *  This variable is different to \c isCurrentAcquisition if
         *  exceptions have been thrown in the start() method. */
        bool isRunning;
        /** True if we have been stopped by the stop() call. */
        bool isStopped;

        /** This function will reset the run counters, compute a
         *  new run size, make us the current acquisition and start
         *  the Andor acquisition. */
        void startRun() throw(std::exception);
        /** This function will wait for the arrival of new images
         *  in the Andor SDK ringbuffer if necessary. */
        void waitForNewImages() throw(std::exception);
        /** This function will wait for the Andor SDK to acknowledge
         *  the end of a run. */
        void waitForEndOfRun() throw(std::exception);

      public:
        /** Constructor receiving the dimensions of the image sequence.
         *  The actual dimensions may be smaller due to a smaller CCD;
         *  please re-check the size with getImageSizeInPixels() after
         *  start().
         *
         *  \param left   Left edge of acquired area, counted from 0
         *  \param right  Right edge of acquired area, counted from 0
         *  \param top    Top edge of acquired area, counted from 0
         *  \param bottom Bottom edge of acquired area, counted from 0
         *  \param numberOfImages Number of images to acquire
         **/
        Acquisition (int left, int right, int top, int bottom,
                     unsigned long numberOfImages) throw ();
        /** Destructor. Will remove the acquisition from Control. */
        virtual ~Acquisition() throw();

        /** Returns the size of a single acquired image in pixels.
         *  May change after first start(). */
        unsigned long getImageSizeInPixels() throw() 
            { return (r-l+1) * (b-t+1); }
        /** Returns the size of a single acquired image in \c char sizes.
         *  May change after first start(). */
        unsigned long getImageSizeInBytes() throw() 
            { return (r-l+1) * (b-t+1) * sizeof(uint16_t); }

        /** Starts this acquisition. */
        void start() throw(std::exception);
        /** Returns true if more images can come from this acquisition.
         *  This result is only reliable if you called locked the mutex
         *  for this instance. */
        bool hasMoreImages() throw() 
            { return (!isStopped) && nextImage < numImages; }
        /** Retrieve the next image from the acquisition into the
         *  provided buffer. This call will block until an image is
         *  available.
         *
         *  \param buffer This vector must be able to hold
         *                getImageSizeInPixels() elements.
         *  \return The index of the retrieved image, starting from 0,
         *          or -1 if an error occured. In the latter case the
         *          data in \a buffer are undefined. */
        long getNextImage(uint16_t *buffer) throw(std::exception);
        /** Stop acquiring images and unregister from Control. */
        void stop() throw(std::exception);
    };
}
#endif
