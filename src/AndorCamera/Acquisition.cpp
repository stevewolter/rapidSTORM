#define ANDORCAMERA_ACQUISITION_CPP
#include "debug.h"

#include <AndorCamera/Acquisition.h>
#include "SDK.h"
#include "Error.h"
#include <cassert>
#include "Readout.h"
#include "AcquisitionMode.h"
#include <limits>
#include <dStorm/error_handler.h>

using namespace std;
using namespace ost;

namespace AndorCamera {

using namespace States;

static const int maxRunSize = 100;
static const int kineticsMode = 3, imageMode = 1;

/* See AndorCamera/Acquisition.h for documentation */
Acquisition::Acquisition (const CameraReference& camera)
: ExclusiveAccessor(camera), control(camera), 
  readout( new ImageReadout( (ImageReadout&)control->readout() ) ),
  acquisitionMode( new AcquisitionModeControl( control->acquisitionMode() ) ),
  haveCamera(false),
  isStopped(false),
  is_acquiring(false),
  status("CameraStatus", "Camera status")
{
    DEBUG("Acquisition constructor");

    status.editable = false;

    Camera::ExclusiveAccessor::replace_on_access( 
        control->readout(), *readout );
    Camera::ExclusiveAccessor::replace_on_access( 
        control->acquisitionMode(), *acquisitionMode );
}

/* See AndorCamera/Acquisition.h for documentation */
Acquisition::~Acquisition() { 
    DEBUG("Destructing acquisition");
    stop();

    DEBUG( "Requesting exclusive accessor to forfeit access" );
    this->Camera::ExclusiveAccessor::forfeit_access();
    DEBUG( "Destructed acquisition" );
}

void Acquisition::got_access() { 
    DEBUG("Acquisition " << this << " got camera access.");
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
    DEBUG("Acquisition " << this << " is starting acquisition");
    request = 
        control->state_machine().ensure_at_least( States::Acquiring,
                                                  StateMachine::Auto );
    DEBUG("Acquisition " << this << " got state machine request rights");
    request->wait_for_fulfillment();
    DEBUG("Acquisition " << this << " brought state machine to acquiring");
    is_acquiring = true;
}
void Acquisition::other_accessor_is_knocking() {
    DEBUG("Acquisition " << this << " got knock.");
}
void Acquisition::forfeit_access() {
    DEBUG("Acquisition " << this << " forfeiting camera access.");
    stop();

    haveCamera = false;

    status = "Released camera access";
    status.erase( control->state_machine().status.value );
    status.push_back( status.value );

    request.reset( NULL );
    DEBUG("Requesting ExclusiveAccessor to forfeit access");
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
    DEBUG("Blocking until on camera");
    Camera::ExclusiveAccessor::wait_for_access();
    got_access();
    DEBUG("Blocked until on camera");
}

/* See AndorCamera/Acquisition.h for documentation */
Acquisition::Fetch Acquisition::getNextImage(uint16_t *buffer) {
    ost::MutexLock lock(mutex);
    if ( ! haveCamera && ! isStopped ) {
        DEBUG("GetNextImage waiting for acquisition start");
        Camera::ExclusiveAccessor::wait_for_access();
        got_access();
        DEBUG("GetNextImage finished wait");
    }

    if ( ! hasMoreImages_unlocked() ) {
        return Fetch( NoMoreImages, 0 );
    } else while (true) {
        waitForNewImages();
        int cur_image = next_image;

        if ( isStopped ) return Fetch( HadError, next_image );

        /* The GetImages function expects and returns ranges. For
         * lack of buffer space, we reduce these to simple images. */
        SDK::Range get;
        get.first = get.second = cur_image;
        SDK::Range r;
        DEBUG("Reading image " << cur_image << " into buffer of size "
                 << getImageSizeInPixels());

        request->check();
        SDK::AcquisitionState get_result = 
            SDK::GetImages16( get, buffer, getImageSizeInPixels(), r );

        /* Check if we are actually finished with this acquisition */
        if ( am_bounded_by_num_images && last_valid_image >= num_images-1 ) {
            DEBUG("Stopping acquisition after last valid image " << last_valid_image << " reached bound of " << num_images);
            stop();
        }

        if ( get_result == SDK::No_New_Images )
            continue;
        else if ( get_result == SDK::Missed_Images ) {
            next_image++;
            return Fetch( HadError, cur_image );
        } else if ( get_result == SDK::New_Images ) {
            if (r.first != next_image)
                throw Error("Expected to read image %i, did read %i.", 
                            next_image, r.first);
            DEBUG("Read image " << cur_image);
            next_image++;

            return Fetch( HaveStored, cur_image );
        }
    }
}

/* See AndorCamera/Acquisition.h for documentation */
void Acquisition::waitForNewImages() {

    /* Already have an image? */
    if (initialized_last_valid_image && next_image <= last_valid_image)
        return;

    request->check();
    SDK::NewImages range = SDK::GetNumberNewImages();
    while ( ! range.haveNew ) {
        request->check();
        DEBUG("Acquisition asks SDK to wait for new images");
        SDK::AcquisitionState rc = SDK::WaitForAcquisition();
        DEBUG("Wait returned " << rc);
        request->check();
        if (rc == SDK::New_Images) {
            range = SDK::GetNumberNewImages();
            DEBUG("New range is " << range.first << " to " 
                     << range.second << ", new is " << range.haveNew);
        } else if (rc == SDK::No_New_Images)  {
            /* This means the event loop in WaitForAcquisition() got
             * terminated early. Retry. */
        }
    }

    last_valid_image = range.second;
    initialized_last_valid_image = true;

#if 0 /* This code is unnecessary. If the image can't be fetched,
         an error will be reported by the getNextImage function. */
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
#endif
}

/* See AndorCamera/Acquisition.h for documentation */
void Acquisition::stop() {
    DEBUG("Stopping acquisition");
    if (!haveCamera || isStopped) return;
    
    ost::MutexLock lock(mutex);

    DEBUG("Calling AbortAcquisition");
    /* Go down from acquisition state. */
    SDK::AbortAcquisition();
    request.reset( NULL );
    is_acquiring = false;

    DEBUG("Finished acquisition stop");
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

bool Acquisition::hasLength() {
    return (am_bounded_by_num_images || (acquisitionMode->select_mode() == Kinetics ||
          acquisitionMode->select_mode() == Fast_Kinetics) );
}

}
