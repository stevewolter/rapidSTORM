#define cimg_use_magick
#define CIMGBUFFER_ANDORCAMERA_VIEWPORTSELECTOR_CPP
#include "debug.h"

#include "ViewportSelector.h"
#include "Acquisition.h"
#include "Readout.h"
#include "AcquisitionMode.h"
#include <limits>
#include "Gain.h"


using namespace std;
using namespace simparm;
using namespace dStorm::input;
using namespace cimg_library;

using dStorm::AndorDirect::CameraPixel;
using dStorm::Pixel;

#define CHECK(x) checkAndorCode( x, __LINE__ )
#define DISP_RESIZE ((detWidth > CImgDisplay::screen_dimx() || \
                      detHeight > CImgDisplay::screen_dimy()) ? 2 : 1)

namespace AndorCamera {
namespace ViewportSelector {

/** Color depth in the viewport selection image. */
static const int imageDepth = 256;

Display::Display( 
    const AndorCamera::CameraReference& cam, 
    Mode mode,
    Config& config,
    Resolution r
)
: simparm::Set("ViewportSelector", "Viewport settings"),
  ost::Thread("Viewport Selector"),
  cam(cam),
  config(config),
  statusBox("CameraStatus", "Camera status"),
  stopAim("StopAimCamera","Leave aiming mode"),
  pause("PauseCamera", "Pause"),
  imageFile("SaveAcquiredImageFile", "Save camera snapshot to"),
  save("SaveAcquiredImage", "Save camera snapshot"),
  aimed( (mode == SelectROI) 
            ? &dynamic_cast<ImageReadout&>(this->cam->readout()) : NULL ),
  paused(false),
  change( new dStorm::Display::Change() ),
  normalization_factor( 1 ),
  lock_normalization( false ),
  resolution( r )
{
    registerNamedEntries();
    this->ost::Thread::start();
}

void Display::registerNamedEntries() {
    receive_changes_from(stopAim.value);
    receive_changes_from(pause.value);
    receive_changes_from(save.value);

    push_back(statusBox);
    push_back(stopAim);
    push_back( cam->gain().emccdGain );
    push_back(pause);
    push_back(imageFile);
    push_back(save);
}

Display::~Display() {
    DEBUG("Destructing ViewportSelector");
    paused = true;
    join();
    DEBUG("Destructed ViewportSelector");
}

std::auto_ptr<dStorm::Display::Change> 
Display::get_changes()
{
    DEBUG("Fetching changes");
    std::auto_ptr<dStorm::Display::Change> other
        ( new dStorm::Display::Change() );

    ost::MutexLock lock(mutex);
    std::swap( other, this->change );
    return other;
}

void Display::notice_closed_data_window() {
    config.delete_active_selector();
}

void Display::configure_camera(Acquisition& acq) 
{
    if ( aimed ) {
        AndorCamera::ImageReadout& ro = acq.getReadoutConfig();
        /* Acquire full detector area. */
        ro.left = aimed->left.min();
        ro.top = aimed->top.min();
        ro.right = aimed->right.max();
        ro.bottom = aimed->bottom.max();
    }

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

dStorm::Display::ResizeChange Display::getSize() const
{
    dStorm::Display::ResizeChange new_size;
    new_size.width = width;
    new_size.height = height;
    new_size.key_size = imageDepth;
    new_size.pixel_size = 
        2.0 / ((resolution.x() + resolution.y()) 
            / cs_units::camera::pixels_per_meter);

    return new_size;
}

void Display::initialize_display() 
{
    if ( aimed != NULL ) {
        width = aimed->right.max() - aimed->left.min() + 1;
        height = aimed->bottom.max() - aimed->top.min() + 1;
    } else {
        ImageReadout& ir = dynamic_cast<ImageReadout&>( cam->readout() );
        width = ir.right() - ir.left() + 1;
        height = ir.bottom() - ir.top() + 1;
    }

    if ( handle.get() == NULL ) {
        dStorm::Display::Manager::WindowProperties props;
        props.name = "Live camera view";
        props.flags.close_window_on_unregister();
        if ( aimed )
            props.flags.notice_drawn_rectangle();
        else
            props.flags.zoom_on_drawn_rectangle();
        props.initial_size = getSize();

        ost::MutexLock lock(mutex);
        handle = dStorm::Display::Manager::getSingleton()
            .register_data_source( props, *this );
        change->do_resize = false;
    } else {
        /* Window already open. The size, however, might have changed if
         * we have no \c aimed member set, so we have to reset it. */
        if ( aimed != NULL ) {
            ost::MutexLock lock(mutex);
            change->do_resize = true;
            change->resize_image = getSize();
        }
    }

    DEBUG("Initialized display");
        
}

void Display::set_resolution( const Resolution& r ) {
    resolution = r;

    change->do_resize = true;
    change->resize_image = getSize();
}

void Display::notice_drawn_rectangle(int l, int r, int t, int b) {
    ost::MutexLock lock(mutex);
    std::cerr << "Noticing rectangle " << l << " " << r << " " << t << " " << b << "\n";
    if ( aimed ) {
        aimed->left = l;
        aimed->right = r;
        aimed->top = t;
        aimed->bottom = b;

        change->do_change_image = true;
        change->image_change.pixels = last_image;
    }
}

void Display::draw_image( const dStorm::AndorDirect::CameraPixel *data) {
    ost::MutexLock lock(mutex);
    /* Determine min and max for norming. */
    dStorm::AndorDirect::CameraPixel minval = data[0], maxval = data[0];
    for ( int i = 1; i < width*height; i++ ) {
        minval = std::min( minval, data[i] );
        maxval = std::max( maxval, data[i] );
    }

    /* Compute normalization and new key. */
    if ( ! lock_normalization ) {
        normalization_factor = imageDepth * 1.0f / (maxval - minval);
        change->change_key.clear();
        dStorm::Display::KeyChange *keys 
            = change->change_key.allocate( imageDepth );
        for (int i = 0; i < imageDepth; i++) {
            keys[i].index = i;
            keys[i].color = dStorm::Pixel(i);
            keys[i].value = i / normalization_factor + minval;
        }
        change->change_key.commit( imageDepth );
    }
    /* Normalize pixels and store result in the ImageChange vector */
    change->image_change.pixels.clear();
    dStorm::Display::Color* pixels = 
        change->image_change.pixels.allocate( width*height );
    for ( int i = 0; i < width*height; i++ ) {
        int n = (data[i] - minval) * normalization_factor;
        pixels[i] = Pixel( std::min( std::max(0, n), imageDepth-1) );
    }
    if ( aimed ) {
        int l = aimed->left(), r = aimed->right(),
            t = aimed->top(), b = aimed->bottom();

        /* Draw the red rectangle that indicates the current acquisition
        * borders */
        for (int v = 0; v < 3; v++) {
            for (int x = l; x <= r; x++)
                pixels[t*width+x] = pixels[b*width+x] = Pixel::Red();
            for (int y = t; y <= b; y++)
                pixels[y*width+l] = pixels[y*width+r] = Pixel::Red();
        }
    }
    change->image_change.pixels.commit( width*height );
    change->do_change_image = true;

    last_image = change->image_change.pixels;
}

void Display::run() throw() {
    try {
        acquire(); 
    } catch (const std::exception& e) {
        std::cerr << "Could not acquire images for aiming view."
                        " Reason: " << e.what() << endl;
        ost::MutexLock lock(mutex);
        handle.reset( NULL );
    } catch (...) {
        std::cerr << "Could not acquire images for aiming view."
                    << endl;
        ost::MutexLock lock(mutex);
        handle.reset( NULL );
    }
    DEBUG("Display acquisition thread finished\n");
}

void Display::acquire() 
{
    /* Start acquisition with unlimited length. Acquisition is stopped
     * by paused variable. */
    DEBUG("Creating acquisition");
    Acquisition acq( cam );
    configure_camera(acq);
    statusBox.push_back( acq.status );
    statusBox.viewable = true;

    DEBUG("Created acquisition");
    acq.start();
    DEBUG("Started acquisition");

    /* Initialize all acquisition fields to determine image size. */
    acq.block_until_on_camera();
    DEBUG("Am on camera");
    statusBox.viewable = false;

    initialize_display();

    /* Allocate buffer for data storage */
    CameraPixel *data = new CameraPixel[ width*height ];

    try {
        while ( ! paused ) {
            /* Acquire image */
            Acquisition::Fetch fetch = acq.getNextImage(data);

            if (fetch.first == Acquisition::NoMoreImages) break;
            if (paused == true || 
                fetch.first == Acquisition::HadError ) continue;

            DEBUG("Drawing image " << fetch.second);
            draw_image( data );
        }

    } catch (...) {
        delete[] data;
        throw;
    }

    delete[] data;
}

void Display::operator()
    (const simparm::Event& e) 
{ 
    if (&e.source == &pause.value && pause.triggered()) {
        pause.untrigger();
        /* No lock  necessary here, since pause is an atomic comparison */
        paused = !paused;
        imageFile.editable = paused;
        save.editable = paused;
        if ( paused )
            join();
        else
            start();
    } else if (&e.source == &save.value && save.triggered()) {
        save.untrigger();
        ost::MutexLock lock( mutex );
        if ( imageFile ) {
            /* TODO */
        }
    } else if (&e.source == &stopAim.value && stopAim.triggered()) {
        stopAim.untrigger();
        config.delete_active_selector();
    }
}

Config::Config(const AndorCamera::CameraReference& cam, Resolution r)
: simparm::Object("SelectImage", "Select image region"),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  cam(cam),
  resolution(r),
  select_ROI("AimCamera","Select ROI"),
  view_ROI("ViewCamera","View ROI only"),
  active_selector_changed(active_selector_mutex)
{
    registerNamedEntries();
}

Config::Config(const Config& c)
: simparm::Object(c),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  cam(c.cam),
  resolution(c.resolution),
  select_ROI(c.select_ROI),
  view_ROI(c.view_ROI),
  active_selector_mutex(),
  active_selector_changed(active_selector_mutex)
{
    registerNamedEntries();
}

Config::~Config() {
}

void Config::operator()(const simparm::Event& e)

{
    if ( &e.source == &select_ROI.value && select_ROI.triggered() ) {
        select_ROI.untrigger();
        make_display( Display::SelectROI );
    } else if ( &e.source == &view_ROI.value && view_ROI.triggered() ) {
        view_ROI.untrigger();
        make_display( Display::ViewROI );
    }
}

void Config::make_display( Display::Mode mode ) 
{
    ost::MutexLock lock( active_selector_mutex );
    active_selector.reset( new Display( cam, mode, *this, resolution ) );
    this->simparm::Node::push_back( *active_selector );

    set_entry_viewability();
    active_selector_changed.signal();
}

void Config::delete_active_selector() 
{
    ost::MutexLock lock( active_selector_mutex );
    active_selector.reset( NULL );

    set_entry_viewability();
    active_selector_changed.signal();
}

void Config::registerNamedEntries() 
{
    receive_changes_from(select_ROI.value);
    receive_changes_from(view_ROI.value);

    push_back(select_ROI);
    push_back(view_ROI);
}

void Config::set_entry_viewability() {
    select_ROI.viewable = (active_selector.get() == NULL);
    view_ROI.viewable = (active_selector.get() == NULL);
}

void Config::set_resolution( const Resolution& r )
{
    resolution = r;
    ost::MutexLock lock(active_selector_mutex);
    if ( active_selector.get() )
        active_selector->set_resolution( r );
}

}
}
