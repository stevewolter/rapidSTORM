#define ANDORCAMERA_ACQUISITION_CPP
#include <AndorCamera/Acquisition.h>
#include "SDK.h"
#include "Error.h"
#include <cassert>
#include "Readout.h"
#include "AcquisitionMode.h"
#include <limits>

using namespace std;
using namespace ost;

namespace AndorCamera {

using namespace States;
using namespace Phases;

static const int maxRunSize = 100;
static const int kineticsMode = 3, imageMode = 1;

/* See AndorCamera/Acquisition.h for documentation */
Acquisition::Acquisition (const CameraReference& camera)
: ExclusiveAccessor(camera), control(camera), 
  readout( new ImageReadout( (ImageReadout&)control->readout() ) ),
  acquisitionMode( new AcquisitionModeControl( control->acquisitionMode() ) ),
  gotStarted(mutex),
  haveCamera(false),
  isStopped(false),
  is_acquiring(false),
  status("CameraStatus", "Camera status")
{
    STATUS("Acquisition constructor");

    status.editable = false;

    Camera::ExclusiveAccessor::replace_on_access( 
        control->readout(), *readout );
    Camera::ExclusiveAccessor::replace_on_access( 
        control->acquisitionMode(), *acquisitionMode );
}

/* See AndorCamera/Acquisition.h for documentation */
Acquisition::~Acquisition() { 
    STATUS("Destructing acquisition");
    stop();
    this->Camera::ExclusiveAccessor::forfeit_access();
    STATUS( "Destructed acquisition" );
}

void Acquisition::got_access() { 
    STATUS("Acquisition " << this << " got camera access.");
    ost::MutexLock lock( mutex );
    haveCamera = true; 
    status.erase( status.value );
    status.push_back( control->state_machine().status.value );

    if ( (acquisitionMode->select_mode() == Kinetics ||
          acquisitionMode->select_mode() == Fast_Kinetics) &&
         acquisitionMode->kinetic_length() > 500 )
    {
        am_bounded_by_num_images = true;
        num_images = acquisitionMode->kinetic_length();
        acquisitionMode->select_mode = Run_till_abort;
    } else
        am_bounded_by_num_images = false;

    next_image = 0;
    initialized_last_valid_image = false;
    STATUS("Acquisition " << this << " is starting acquisition");
    control->state_machine().ensure_at_least( States::Acquiring );
    STATUS("Acquisition " << this << " started acquisition");
    is_acquiring = true;

    gotStarted.signal();
}
void Acquisition::other_accessor_is_knocking() {
    STATUS("Acquisition " << this << " got knock.");
}
void Acquisition::forfeit_access() {
    STATUS("Acquisition " << this << " forfeiting camera access.");
    stop();

    haveCamera = false;

    status = "Released camera access";
    status.erase( control->state_machine().status.value );
    status.push_back( status.value );

    control->state_machine().ensure_at_most( lower_state( States::Acquiring ) );
    Camera::ExclusiveAccessor::forfeit_access();
}

/* See AndorCamera/Acquisition.h for documentation */
void Acquisition::start() {
    isStopped = false;
    status = "Waiting for camera";

    Camera::ExclusiveAccessor::request_access();
}

unsigned int Acquisition::getLength() { 
    if ( acquisitionMode->select_mode() == Run_till_abort )
      if ( am_bounded_by_num_images )
        return num_images;
      else
        return std::numeric_limits<unsigned int>::max();
    else if ( acquisitionMode->select_mode() == Kinetics ||
              acquisitionMode->select_mode() == Fast_Kinetics )
        return acquisitionMode->kinetic_length();
    else
        return 1;
}

bool Acquisition::hasMoreImages_unlocked() {
    if ( isStopped ) return false;
    return (next_image < getLength());
}

bool Acquisition::hasMoreImages() {
    ost::MutexLock lock(mutex);
    return hasMoreImages_unlocked();
}

void Acquisition::block_until_on_camera() {
    ost::MutexLock lock(mutex);
    while ( ! is_acquiring )
        gotStarted.wait();
}

/* See AndorCamera/Acquisition.h for documentation */
long Acquisition::getNextImage(uint16_t *buffer) {
    ost::MutexLock lock(mutex);
    while ( ! haveCamera && ! isStopped ) {
        PROGRESS("GetNextImage waiting for acquisition start");
        gotStarted.wait();
        PROGRESS("GetNextImage finished wait");
    }

    if ( ! hasMoreImages_unlocked() ) {
        return -1;
    } else {
        waitForNewImages();

        int cur_image = next_image;

        /* The GetImages function expects and returns ranges. For
         * lack of buffer space, we reduce these to simple images. */
        SDK::Range get;
        get.first = get.second = cur_image;
        SDK::Range r;
        PROGRESS("Reading buffer of size " << getImageSizeInPixels());
        SDK::GetImages16( get, buffer, getImageSizeInPixels(), r );
        if (r.first != next_image)
            throw Error("Expected to read image %i, did read %i.", 
                        next_image, r.first);
        PROGRESS("Read image " << cur_image);
        next_image++;

        return cur_image;
    }
}

/* See AndorCamera/Acquisition.h for documentation */
void Acquisition::waitForNewImages() {

    /* Already have an image? */
    if (initialized_last_valid_image && next_image <= last_valid_image)
        return;

    SDK::NewImages range = SDK::GetNumberNewImages();
    while ( ! range.haveNew ) {
        PROGRESS("Acquisition asks SDK to wait for new images");
        SDK::AcquisitionState rc = SDK::WaitForAcquisition();
        PROGRESS("Acquisition returned from SDK-wait");
        if (rc == SDK::New_Images)
            range = SDK::GetNumberNewImages();
        else if (rc == SDK::No_New_Images)  {
            /* This means the event loop in WaitForAcquisition() got
             * terminated early. Retry. */
        }
    }

    last_valid_image = range.second;
    initialized_last_valid_image = true;

    /* Stop acquisition if we are managing a long kinetic run. */
    if ( am_bounded_by_num_images && last_valid_image >= num_images-1 )
        stop();

    /* Naturally, we expect the next image in the run to be the first
     * in the ring buffer. But, if we were really, really slow, it might
     * have been overwritten; in that case, we output a warning.
     *
     * If we did miss any images in the ring buffer, we will acquire more
     * images at the run's end. */
    if (next_image < range.first) {
        cerr << "AndorCamera warning: Skipping from " << int(next_image)
             << " to " << last_valid_image << endl;
        next_image = range.first;
    }
}

/* See AndorCamera/Acquisition.h for documentation */
void Acquisition::stop() {
    PROGRESS("Stopping acquisition");
    if (!haveCamera || isStopped) return;
    
    ost::MutexLock lock(mutex);

    PROGRESS("Calling AbortAcquisition");
    /* Go down from acquisition state. */
    SDK::AbortAcquisition();
    control->state_machine().ensure_at_most ( lower_state( Acquiring ) );
    is_acquiring = false;

    STATUS("Finished acquisition stop");
    isStopped = true;
}

unsigned long Acquisition::getImageSizeInPixels() { 
    return  getWidth() * getHeight();
}

unsigned long Acquisition::getImageSizeInBytes() { 
    return getImageSizeInPixels() * sizeof(uint16_t);
}

unsigned int Acquisition::getWidth() {
    return (readout->right()-readout->left()+1);
}

unsigned int Acquisition::getHeight() {
    return (readout->bottom()-readout->top()+1); 
}

}
