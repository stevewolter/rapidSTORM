#define cimg_use_magick
#define CIMGBUFFER_ANDORCAMERA_VIEWPORTSELECTOR_CPP
#include "ViewportSelector.h"
#include <AndorCamera/Acquisition.h>
#include <AndorCamera/Readout.h>
#include <AndorCamera/AcquisitionMode.h>
#include <limits>

using namespace std;
using namespace AndorCamera;
using namespace simparm;
using namespace CImgBuffer;
using namespace cimg_library;

#define CHECK(x) checkAndorCode( x, __LINE__ )
#define DISP_RESIZE ((detWidth > CImgDisplay::screen_dimx() || \
                      detHeight > CImgDisplay::screen_dimy()) ? 2 : 1)

namespace dStorm {
namespace AndorDirect {

/** Helper class for Viewport selector that runs its
 *  acquire() method concurrently. */
class ImageAcquirer : public ost::Thread {
  private:
    ViewportSelector &vps;
  public:
    ImageAcquirer(ViewportSelector &vps) 
        : ost::Thread("Image acquirer"), vps(vps) {}
    ~ImageAcquirer() { join(); }
    void run() throw() { 
        try {
            vps.acquire(); 
        } catch (const std::exception& e) {
            std::cerr << "Could not acquire images for aiming view."
                         " Reason: " << e.what() << endl;
        } catch (...) {
            std::cerr << "Could not acquire images for aiming view."
                      << endl;
        }
    }
};

/** Number of color channels in the viewport selection image. */
static const int colorChannels = 3;

ViewportSelector::ViewportSelector(
    const AndorCamera::CameraReference& cam,
    ImageReadout& c,
    ViewportSelector::Config &config
)
: simparm::Set("ViewportSelector", "Viewport settings"),
  ost::Thread("Viewport Selector"),
  cam(cam),
  left(c.left), right(c.right), 
  top(c.top), bottom(c.bottom),
  config(config),
  needMoreImages(true),
  close_viewer(false),
  display_was_initialized(*this),
  paused(false)
{
    receive_changes_from(config.pause);
    receive_changes_from(config.save);
}

ViewportSelector::~ViewportSelector() {
    PROGRESS("Destructing ViewportSelector");
    enterMutex();
    if (display.get() != NULL) display->close();
    leaveMutex();

    join();
    PROGRESS("Destructed ViewportSelector");
}

void ViewportSelector::run() throw()
{
  try {
    bool has_been_open = false;

    haveMoreImages= true;
    imageAcquirer.reset( new ImageAcquirer(*this) );
    imageAcquirer->start();
    /* Last valid mouse position in X/Y coordinates. */
    unsigned int lastMX = 0, lastMY = 0;

    enterMutex();
    while ( display.get() == NULL )
        display_was_initialized.wait();
    CImgDisplay &d = *display;

    while ( haveMoreImages && !close_viewer && 
            (!has_been_open || !d.is_closed) ) 
    {
        bool move = false;
        if (!d.is_closed) has_been_open = true;
        if (d.mouse_x >= 0) lastMX = d.mouse_x;
        if (d.mouse_y >= 0) lastMY = d.mouse_y;
        if (d.is_event) {
            /* move is set to true if any mouse button was pressed */
            for ( int i = 0; i < 512; i++)
                if (d.buttons[i] == 1) {
                    move = true;
                    break;
                }
        }
        d.flush();
        leaveMutex();

        if (move) {
            /* Determine which corner of the rectangle is closest. */
            int mx = lastMX * mapFac, my = lastMY * mapFac;
            int middleX = (right() + left()) / 2;
            int middleY = (top() + bottom()) / 2;
            /* Set closest corner to last known mouse position. */
            if (mx < middleX) left = mx; else right = mx;
            if (my < middleY) top = my; else bottom = my;
        }
        cimg::wait(30);
        enterMutex();
    }
    needMoreImages = false;
    leaveMutex();

    PROGRESS("Joining image acquirer");
    imageAcquirer->join();
    PROGRESS("Joined image acquirer");
    config.viewport_selector_finished();
    PROGRESS("Signalled finishment to config");
  } catch (const std::exception& e) {
    std::cerr << "Failed to update aiming view. Reason: "
              << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Failed to update aiming view." << std::endl;
  }
}

void ViewportSelector::configure_camera(Acquisition& acq) 
{
    /* Acquire full detector area. */
    acq.getReadoutConfig().left = acq.getReadoutConfig().left.min();
    acq.getReadoutConfig().top = acq.getReadoutConfig().top.min();
    acq.getReadoutConfig().right = acq.getReadoutConfig().right.max();
    acq.getReadoutConfig().bottom = acq.getReadoutConfig().bottom.max();

    /* Acquire with 10 images / second */
    acq.getAcquisitionModeControl().desired_kinetic_cycle_time = 0.1;
    acq.getAcquisitionModeControl().desired_accumulate_cycle_time = 
        cam->acquisitionMode().desired_accumulate_cycle_time();
    acq.getAcquisitionModeControl().desired_exposure_time = 
        cam->acquisitionMode().desired_exposure_time();

    /* Acquire eternally. */
    acq.getAcquisitionModeControl().select_mode = 
        AndorCamera::Run_till_abort;
}

void ViewportSelector::initialize_display() 
{
    LOCKING("Acquiring viewport lock");
    enterMutex();
    LOCKING("Acquired viewport lock");
    detWidth = (right.max()+1); 
    detHeight = (bottom.max()+1);
    mapFac = (DISP_RESIZE);
    display.reset( new CImgDisplay(detWidth/mapFac, detHeight/mapFac, 
          "Camera calibration", 1));
    tempImage.resize(detWidth, detHeight, 1, colorChannels);

    display_was_initialized.signal();
    leaveMutex();
    PROGRESS("Initialized display");
        
}

void ViewportSelector::draw_image( const CameraPixel *data) {
    /* Copy the recorded signal into every color channel. */
    for (int i = 0; i < colorChannels; i++)
        memcpy( tempImage.ptr(0,0,0,i), data, 
                tempImage.width * tempImage.height * sizeof(CameraPixel) );
    int max, min = tempImage.minmax(max);
    static int tenth_image = 0;

    if ( tenth_image == 0 ) {
        std::stringstream dr;
        dr << min << " to " << max << "\n";
        config.dynamic_range = dr.str();
    }
    tenth_image= (tenth_image == 9) ? 0 : tenth_image+1;

    /* Find coordinates close to the true border which are displayable 
     * in the reduced view. */
    LOCKING("Acquiring mutex for determining borders");
    enterMutex();
    LOCKING("Acquired mutex for determining borders");
    int l = (left() / mapFac) * mapFac, 
        r = (right() / mapFac) * mapFac,
        t = (top() / mapFac) * mapFac,
        b = (bottom() / mapFac) * mapFac;
    leaveMutex();

    /* Draw the red rectangle that indicates the current acquisition
     * borders */
    for (int v = 0; v < colorChannels; v++) {
        for (int x = l; x <= r; x++) {
            tempImage(x, t, 0, v) = (v) ? min : max;
            tempImage(x, b, 0, v) = (v) ? min : max;
        }
        for (int y = t; y <= b; y++) {
            tempImage(l, y, 0, v) = (v) ? min : max;
            tempImage(r, y, 0, v) = (v) ? min : max;
        }
    }
}

void ViewportSelector::acquire() 
{
    /* Start acquisition with unlimited length. Acquisition is stopped
     * by needMoreImages variable. */
    PROGRESS("Creating acquisition");
    Acquisition acq( cam );
    configure_camera(acq);
    config.statusBox.push_back( acq.status );
    config.statusBox.viewable = true;

    PROGRESS("Created acquisition");
    try {
        acq.start();
    } catch (const std::exception & e) {
        cerr << "Could not start image acquisition: " << e.what() << endl;
        haveMoreImages = false;
        return;
    }
    PROGRESS("Started acquisition");

    /* Initialize all acquisition fields to determine image size. */
    acq.block_until_on_camera();
    PROGRESS("Am on camera");
    config.statusBox.viewable = false;
    config.stopAim.viewable = true;
    config.pause.viewable = true;

    initialize_display();
    config.dynamic_range.viewable = true;

    /* Allocate buffer for data storage */
    CameraPixel *data = new CameraPixel[ detWidth * detHeight ];

    while ( needMoreImages ) {
        long index;
        /* Initialize first data elements. Was used for debugging and
         * am afraid code will break if removed. */
        data[0] = data[1] = data[2] = 0;
        /* Acquire image */
        try {
            LOCKING("Trying to acquire image from camera");
            index = acq.getNextImage(data);
            LOCKING("Got image " << index << ", am " 
                    << ((paused) ? "" : "not ") << "paused");
        } catch (const std::exception &e) {
            cerr << "Image acquisition failed:" << e.what() << endl;
            break;
        }
        if (index == -1) break;
        if (paused == true) continue;

        draw_image( data );

        enterMutex();
        *display << tempImage;
        leaveMutex();
    }

    config.dynamic_range.viewable = false;
    delete[] data;
    haveMoreImages = false;
}

void ViewportSelector::operator()
    (Node &src, Cause, Node *) 
 
{ 
    if (&src == &config.pause && config.pause.triggered()) {
        config.pause.untrigger();
        /* No lock  necessary here, since pause is an atomic comparison */
        paused = !paused;
        config.imageFile.viewable = paused;
        config.save.viewable = paused;
    } else if (&src == &config.save && config.save.triggered()) {
        config.save.untrigger();
        /* No lock necessary here, since change in the image data is not
         * critical to the write process */
        if ( config.imageFile ) {
            try {
                CImg<uint8_t> brokenDown = tempImage.get_normalize(0,255);
                brokenDown.save( config.imageFile().c_str() );
            } catch (const cimg_library::CImgException &e) {
                cerr << "CImg: " << e.message << endl;
            } catch (const std::exception &e) {
                cerr << "CImg: " << e.what() << endl;
            }
        }
    }
}

void ViewportSelector::terminate() {
    close_viewer = true;
}

ViewportSelector::Config::Config(const AndorCamera::CameraReference& cam)
 :
  simparm::Object("SelectImage", "Select image region"),
  cam(cam),
  aim("AimCamera","Take aim"),
  statusBox("CameraStatus", "Camera status"),
  dynamic_range("DynamicRange", "Dynamic range of image"),
  stopAim("StopAimCamera","Leave aiming mode"),
  pause("PauseCamera", "Pause"),
  imageFile("SaveAcquiredImageFile", "Save camera snapshot to"),
  save("SaveAcquiredImage", "Save camera snapshot"),
  noActiveSelector(activeSelectorMutex),
  activeSelector(NULL)
{
    aim.setUserLevel(Entry::Beginner);
    stopAim.setUserLevel(Entry::Beginner);
    pause.setUserLevel(Entry::Beginner);
    imageFile.setUserLevel(Entry::Beginner);
    save.setUserLevel(Entry::Beginner);

    dynamic_range.viewable = false;
    dynamic_range.editable = false;

    statusBox.setViewable( false );
    stopAim.setViewable(false);
    pause.setViewable(false);
    imageFile.setViewable(false);
    save.setViewable(false);

    registerNamedEntries();
}

ViewportSelector::Config::Config(const ViewportSelector::Config::Config& c)

: simparm::Node(c), simparm::Object(c),
  simparm::Node::Callback(),
  cam(c.cam),
  aim(c.aim),
  statusBox(c.statusBox),
  dynamic_range(c.dynamic_range),
  stopAim(c.stopAim),
  pause(c.pause),
  imageFile(c.imageFile),
  save(c.save),
  noActiveSelector(activeSelectorMutex),
  activeSelector(NULL)
{
    registerNamedEntries();
}

ViewportSelector::Config::~Config() {
    stopAiming();
}

void ViewportSelector::Config::operator()(Node &src, Cause c, Node*)

{
    if ( c != ValueChanged) return;
    if ( &src == &aim && aim.triggered() && aim.viewable() ) {
        aim.untrigger();
        startAiming();
    } else if ( &src == &stopAim && stopAim.triggered() && stopAim.viewable() ) {
        stopAim.untrigger();
        stopAiming();
    }
}

void ViewportSelector::Config::registerNamedEntries() 
{
    receive_changes_from(aim);
    receive_changes_from(stopAim);

    register_entry(&aim);
    register_entry(&statusBox);
    register_entry(&dynamic_range);
    register_entry(&stopAim);
    register_entry(&pause);
    register_entry(&imageFile);
    register_entry(&save);
}

void ViewportSelector::Config::startAiming() {
    aim.setViewable( false );

    AndorCamera::ImageReadout* imageReadout =
        dynamic_cast<AndorCamera::ImageReadout*>( &cam->readout() );
    if ( imageReadout ) {
        activeSelector =
            new ViewportSelector(cam, *imageReadout, *this);
        activeSelector->detach();
    } else 
        std::cerr << "Wrong acquisition type in viewport selector.\n";
}

void ViewportSelector::Config::stopAiming() {
    ost::MutexLock activeSelectorLock( activeSelectorMutex );
    if ( activeSelector != NULL )
        activeSelector->terminate();
    while ( activeSelector != NULL )
        noActiveSelector.wait();
}

void ViewportSelector::Config::viewport_selector_finished() {
    aim.setViewable( true );
    stopAim.setViewable( false );
    pause.setViewable( false );
    imageFile.setViewable( false );
    save.setViewable( false );

    ost::MutexLock lock(activeSelectorMutex);
    activeSelector = NULL;
    noActiveSelector.signal();
}

}
}
