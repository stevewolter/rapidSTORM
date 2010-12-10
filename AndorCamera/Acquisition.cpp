#define ANDORCAMERA_ACQUISITION_CPP
#include <AndorCamera/Acquisition.h>
#include <AndorCamera/Control.h>
#include "SDK.h"
#include "Error.h"

#define CHECK(x) { int line = __LINE__; checkAndorCode( x, line ); }

using namespace std;
using namespace AndorCamera;
using namespace ost;

static const int maxRunSize = 100;
static const int kineticsMode = 3, imageMode = 1;

/* See AndorCamera/Acquisition.h for documentation */
Acquisition::Acquisition (int left, int right, int top, int bottom,
                     unsigned long numberOfImages) throw () 
: l(left), r(right), t(top), b(bottom), 
  numImages(numberOfImages),
  isCurrentAcquisition(false), isRunning(false), isStopped(false)
{
}

/* See AndorCamera/Acquisition.h for documentation */
Acquisition::~Acquisition() throw() { 
    STATUS("Destructing acquisition");
    try { stop(); } catch(const std::exception& e) {
        /** \note This destructor will stop its acquisition, but
         *        silently catch and ignore any exceptions thrown
         *        in that process. Enable debug output to see these. */
        STATUS("Exception in Acquisition destructor: " << e.what());
    } 
    STATUS("Destructed acquisition");
}

/* See AndorCamera/Acquisition.h for documentation */
void Acquisition::startRun() throw(std::exception) {
    MutexLock lock(*this);

    assert ( numImages >= nextImage );

    unsigned long imagesLeft = numImages - nextImage;
    runSize = min<unsigned long>(maxRunSize, imagesLeft);
    imageInRun = 0;
    lastValidImage = -1;

    if (runSize == 0) {
        /* This means we have no images left. Stop. */
    } else {
        PROGRESS("Setting current acquisition");
        control->setCurrentAcquisition(this);
        PROGRESS("Set current acquisition");
        isCurrentAcquisition = true;

        /* Make sure that the dimensions of the acquisition
         * are not too large. */
        r = min<int>(r, control->getDetectorSize().w-1);
        l = min<int>(r, l);
        b = min<int>(b, control->getDetectorSize().h-1);
        t = min<int>(t, b);

        /* Enable kinetics mode if necessary and useful. */
        if ( runSize > 1 ) {
            PROGRESS("Kinetics mode");
            AndorCamera::SetAcquisitionMode( Kinetics );
            AndorCamera::SetNumberKinetics( runSize );
        } else {
            PROGRESS("Image mode");
            SetAcquisitionMode( Image );
        }

        PROGRESS("Image binning");
        SetImageNoBinning(l, r, t, b);
        PROGRESS("Starting acquisition");
        StartAcquisition();
        isRunning = true;
    }
}

/* See AndorCamera/Acquisition.h for documentation */
void Acquisition::start() throw(std::exception) {
    MutexLock lock(*this);

    isStopped = false;
    nextImage = 0;

    startRun();
}

/* See AndorCamera/Acquisition.h for documentation */
long Acquisition::getNextImage(uint16_t *buffer) throw(std::exception) {
    MutexLock lock(*this);

    if ( ! hasMoreImages() ) {
        return -1;
    } else {
        if (!isRunning) startRun();
        waitForNewImages();

        int curImage = nextImage;

        /* The GetImages function expects and returns ranges. For
         * lack of buffer space, we reduce these to simple images. */
        Range get;
        get.first = get.second = imageInRun;
        Range r;
        GetImages16( get, buffer, getImageSizeInPixels(), r );
        if (r.first != imageInRun)
            throw Error("Expected to read image %i, did read %i.", 
                        imageInRun, r.first);
        PROGRESS("Read image " << nextImage);
        imageInRun++;
        nextImage++;

        if (imageInRun >= runSize) {
            waitForEndOfRun();
            startRun();
        }
        return curImage;
    }
}

/* See AndorCamera/Acquisition.h for documentation */
void Acquisition::waitForNewImages() throw(std::exception) {
    MutexLock lock(*this);

    /* Already have an image? */
    if (imageInRun <= lastValidImage) return;

    NewImages range = GetNumberNewImages();
    while ( ! range.haveNew ) {
        int rc = WaitForAcquisition();
        if (rc == DRV_SUCCESS)
            range = GetNumberNewImages();
        else if (rc == DRV_NO_NEW_DATA)  {
            /* This means the event loop in WaitForAcquisition() got
             * terminated early - or the acquisition stopped. We check
             * which of the two it is. */
            int status;
            CHECK( GetStatus(&status) );
            if (status != DRV_ACQUIRING)
                startRun();
        } else
            /* Only DRV_SUCCESS and DRV_NO_NEW_DATA are valid return
             * values, but better safe than sorry. */
            CHECK(rc);
    }

    lastValidImage = range.second;
    /* Naturally, we expect the next image in the run to be the first
     * in the ring buffer. But, if we were really, really slow, it might
     * have been overwritten; in that case, we output a warning.
     *
     * If we did miss any images in the ring buffer, we will acquire more
     * images at the run's end. */
    if (imageInRun < range.first) {
        cerr << "AndorCamera warning: Skipping from " << imageInRun
             << " to " << lastValidImage << endl;
        imageInRun = range.first;
    }
}

/* See AndorCamera/Acquisition.h for documentation */
void Acquisition::waitForEndOfRun() throw(std::exception) {
    MutexLock lock(*this);

    if (!isRunning) return;

    int status;
    CHECK( GetStatus( &status ) );

    while (status == DRV_ACQUIRING) {
        WaitForAcquisition();
        CHECK( GetStatus( &status ) );
    }

    if ( ! hasMoreImages() )  {
        /* This is the somehow natural place to free the memory the Andor
         * driver allocated for the acquisition. We can't guarantee that
         * there won't be another acquisition that needs the memory, thus
         * it might be beneficial to remove this line. It is not necessary
         * in any way. */
        CHECK( FreeInternalMemory() );
    }

    isRunning = false;
}

/* See AndorCamera/Acquisition.h for documentation */
void Acquisition::stop() throw(std::exception) {
    PROGRESS("Stopping acquisition");
    MutexLock lock(*this);

    if (isRunning) {
        CHECK( AbortAcquisition() );
        waitForEndOfRun();
    }
    STATUS("Removing acquisition pointer");
    if (isCurrentAcquisition) {
        isCurrentAcquisition = false;
        control->setCurrentAcquisition(NULL);
    }
    STATUS("Finished acquisition stop");
    isStopped = true;
}
