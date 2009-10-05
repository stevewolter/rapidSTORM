/** \file 
 *  This file contains the AndorCamera::Acquisition class declaration. */
#ifndef ANDORCAMERA_ACQUISITION_H
#define ANDORCAMERA_ACQUISITION_H

#include <AndorCamera/Camera.h>
#include <AndorCamera/System.h>
#include <stdint.h>
#include <time.h>
#include <cc++/thread.h>

namespace AndorCamera {
    class ImageReadout;
    class AcquisitionModeControl;

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
     *  the Camera class; in the latter case, getNextImage() will
     *  unexpectedly return -1.
     *
     *  To save memory, the Acquisition class internally divides
     *  its operation into runs of \c runSize images.
     *  */
    class Acquisition : private Camera::ExclusiveAccessor {
      private:
        CameraReference control;
        std::auto_ptr<ImageReadout> readout;
        std::auto_ptr<AcquisitionModeControl> acquisitionMode;
        ost::Mutex mutex;
        ost::Condition gotStarted;

        /** The next image in the acquisition sequence */
        unsigned long next_image;
        /** The total number of images this acquisition should acquire.
         *  This variable is only set when acquisitionMode->select_mode
         *  is set to Run_till_abort. */
        unsigned long num_images;
        /** Indicates whether num_images is set. */
        bool am_bounded_by_num_images;
        /** The last image that is present in the Andor SDK ring buffer.*/
        unsigned long last_valid_image;
        bool initialized_last_valid_image;
        /** True while we have camera access. */
        bool haveCamera;
        /** True if we have been stopped by the stop() call. */
        bool isStopped;
        /** True while the camera is acquiring data. */
        bool is_acquiring;

        /** This function will wait for the arrival of new images
         *  in the Andor SDK ringbuffer if necessary. */
        void waitForNewImages();

        void got_access();
        void other_accessor_is_knocking();
        void forfeit_access();

        bool hasMoreImages_unlocked();

      public:
        /** Constructor receiving the dimensions of the image sequence.
         *  The actual dimensions may be smaller due to a smaller CCD;
         *  please re-check the size with getImageSizeInPixels() after
         *  start().
         *
         *  \param camera Camera to acquire from
         *  \param numberOfImages Number of images to acquire
         **/
        Acquisition (const CameraReference& camera);
        /** Destructor. Will remove the acquisition from Camera. */
        virtual ~Acquisition();

        /** @return A reference to the ImageReadout structure that defines the
         *          readout parameters for the camera. */
        ImageReadout& getReadoutConfig() { return *readout; }
        /** @return A reference to the AcquisitionModeControl structure that
         *          defines the acquisition parameters for the camera. */
        AcquisitionModeControl& getAcquisitionModeControl() 
            { return *acquisitionMode; }

        /** Returns the size of a single acquired image in pixels.
         *  May change after first start(). */
        unsigned long getImageSizeInPixels();
        /** Returns the size of a single acquired image in \c char sizes.
         *  May change after first start(). */
        unsigned long getImageSizeInBytes();

        /** @return Width of the acquired image in pixels. */
        unsigned int getWidth();
        /** @return Height of the acquired image in pixels. */
        unsigned int getHeight();
        /** @return Number of images to capture. */
        unsigned int getLength();

        /** Starts this acquisition. */
        void start();
        /** Returns true if more images can come from this acquisition. */
        bool hasMoreImages();
        /** This function will block until the camera went to the acquiring
         *  state. This ensures the information in the configuration items
         *  and the width, height and length information is accurate. */
        void block_until_on_camera();
        /** Retrieve the next image from the acquisition into the
         *  provided buffer. This call will block until an image is
         *  available.
         *
         *  \param buffer This vector must be able to hold
         *                getImageSizeInPixels() elements.
         *  \return The index of the retrieved image, starting from 0,
         *          or -1 if an error occured. In the latter case the
         *          data in \a buffer are undefined. */
        long getNextImage(uint16_t *buffer);
        /** Stop acquiring images and unregister from Camera. */
        void stop();

        simparm::StringEntry status;
    };
}
#endif
