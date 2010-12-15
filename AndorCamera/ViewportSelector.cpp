#define CIMGBUFFER_ANDORCAMERA_VIEWPORTSELECTOR_CPP
#include "debug.h"

#include "ViewportSelector.h"
#include "Acquisition.h"
#include "Readout.h"
#include "AcquisitionMode.h"
#include <limits>
#include "Gain.h"
#include <boost/units/io.hpp>

#include <dStorm/Image_impl.h>
#include <dStorm/helpers/exception.h>
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/Context_impl.h>
#include <simparm/ChoiceEntry_Impl.hh>

using namespace std;
using namespace simparm;
using namespace dStorm::input;

using AndorCamera::CameraPixel;
using dStorm::Pixel;

#define CHECK(x) checkAndorCode( x, __LINE__ )

namespace AndorCamera {
namespace ViewportSelector {

/** Color depth in the viewport selection image. */
static const int imageDepth = 256;

Display::Display( 
    const CameraReference& cam, 
    Mode mode,
    Config& config,
    const Context& context
)
: simparm::Set("ViewportSelector", "Viewport settings"),
  ost::Thread("Viewport Selector"),
  cam(cam),
  config(config),
  context(context),
  statusBox("CameraStatus", "Camera status"),
  stopAim("StopAimCamera","Leave aiming mode"),
  pause("PauseCamera", "Pause"),
  imageFile("SaveAcquiredImageFile", "Save camera snapshot to"),
  save("SaveAcquiredImage", "Save camera snapshot"),
  aimed( (mode == SelectROI) 
            ? &dynamic_cast<ImageReadout&>(this->cam->readout()) : NULL ),
  paused(false),
  change( new dStorm::Display::Change(1) ),
  normalization_factor( 0, 1 ),
  lock_normalization( false ),
  redeclare_key(false)
{
    imageFile.editable = false;
    save.editable = false;
    imageFile = context.output_basename + ".jpg";

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
        ( new dStorm::Display::Change(1) );

    ost::MutexLock lock(mutex);
    std::swap( other, this->change );
    DEBUG("Fetched changes");
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
    acq.getAcquisitionModeControl().desired_kinetic_cycle_time 
        = 0.1f * boost::units::si::seconds;
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
    new_size.size = size;
    new_size.keys.push_back( 
        dStorm::Display::KeyDeclaration("ADC", "A/D counts", imageDepth) );
    new_size.keys.back().can_set_lower_limit = true;
    new_size.keys.back().can_set_upper_limit = true;
    new_size.keys.back().lower_limit = "";
    new_size.keys.back().upper_limit = "";
    if ( context.has_info_for<CamImage>() && context.get_info_for<CamImage>().resolution.is_set() )
        new_size.pixel_size = *context.get_info_for<CamImage>().resolution;

    return new_size;
}

void Display::initialize_display() 
{
    if ( aimed != NULL ) {
        size.x() = aimed->right.max() - aimed->left.min() + 1 * cs_units::camera::pixel;
        size.y() = aimed->bottom.max() - aimed->top.min() + 1 * cs_units::camera::pixel;
    } else {
        ImageReadout& ir = dynamic_cast<ImageReadout&>( cam->readout() );
        size.x() = ir.right() - ir.left() + 1 * cs_units::camera::pixel;
        size.y() = ir.bottom() - ir.top() + 1 * cs_units::camera::pixel;
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

        DEBUG("Initializing display handle");
        ost::MutexLock lock(mutex);
        DEBUG("Got lock for display handle");
        handle = dStorm::Display::Manager::getSingleton()
            .register_data_source( props, *this );
        change->do_resize = false;
        change->do_clear = true;
        change->clear_image.background = 0;
    } else {
        /* Window already open. The size, however, might have changed if
         * we have no \c aimed member set, so we have to reset it. */
        if ( aimed != NULL ) {
            ost::MutexLock lock(mutex);
            change->do_resize = true;
            change->resize_image = getSize();
            change->do_clear = true;
            change->clear_image.background = 0;
        }
    }

    DEBUG("Initialized display");
        
}

void Display::context_changed() {
    DEBUG("Setting resolution " << *context.resolution);
    imageFile = context.output_basename + ".jpg";

    change->do_resize = true;
    change->resize_image = getSize();
}

void Display::notice_drawn_rectangle(int l, int r, int t, int b) {
    ost::MutexLock lock(mutex);
    if ( aimed ) {
        aimed->left = l * cs_units::camera::pixel;
        aimed->right = r * cs_units::camera::pixel;
        aimed->top = t * cs_units::camera::pixel;
        aimed->bottom = b * cs_units::camera::pixel;

        change->do_change_image = true;
        change->image_change.new_image = last_image;
    }
}

void Display::draw_image( const CamImage& data) {
    ost::MutexLock lock(mutex);

    /* Compute normalization and new key. */
    if ( ! lock_normalization ) {
        if ( ! lower_user_limit.is_set() || ! upper_user_limit.is_set() ) {
            normalization_factor = data.minmax();
            redeclare_key = true;
        }
    }
    
    if ( redeclare_key ) {
        if ( lower_user_limit.is_set() )
            normalization_factor.first = (*lower_user_limit) / cs_units::camera::ad_count;
        if ( upper_user_limit.is_set() )
            normalization_factor.second = (*upper_user_limit) / cs_units::camera::ad_count;

        change->changed_keys.front().clear();
        dStorm::Display::KeyChange *keys 
            = change->changed_keys.front().allocate( imageDepth );
        for (int i = 0; i < imageDepth; i++) {
            keys[i].index = i;
            keys[i].color = dStorm::Pixel(i);
            keys[i].value = i * 1.0 * (normalization_factor.second - normalization_factor.first)
                / imageDepth + normalization_factor.first;
        }
        change->changed_keys.front().commit( imageDepth );
        redeclare_key = false;
    }
    /* Normalize pixels and store result in the ImageChange vector */
    dStorm::Image<dStorm::Pixel,2>& img = change->image_change.new_image;
    img = data.normalize<dStorm::Pixel>(normalization_factor);
    DEBUG("Max for normalized image is " << img.minmax().first);

    if ( aimed ) {
        int l = aimed->left() / cs_units::camera::pixel,
            r = aimed->right() / cs_units::camera::pixel,
            t = aimed->top() / cs_units::camera::pixel,
            b = aimed->bottom() / cs_units::camera::pixel;

        /* Draw the red rectangle that indicates the current acquisition
        * borders */
        for (int v = 0; v < 3; v++) {
            for (int x = l; x <= r; x++)
                img(x,t) = img(x,b) = Pixel::Red();
            for (int y = t; y <= b; y++)
                img(l,y) = img(r,y) = Pixel::Red();
        }
    }
    change->do_change_image = true;

    last_image = img;
}

void Display::run() throw() {
    try {
        acquire(); 
        return;
    } catch (const dStorm::runtime_error& e) {
        simparm::Message m( e.get_message("Could not acquire images for aiming view") );
        send(m);
    } catch (const std::exception& e) {
        simparm::Message m( "Could not acquire images for aiming view", e.what() );
        send(m);
    } catch (...) {
        simparm::Message m( "Could not acquire images for aiming view", 
                            "Unknown error occured while acquiring images for aiming view." );
        send(m);
    }
    handle.reset( NULL );
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
    CamImage img( size, 0 * cs_units::camera::frame );

    while ( ! paused ) {
        /* Acquire image */
        Acquisition::Fetch fetch = acq.getNextImage(img.ptr());

        if (fetch.first == Acquisition::NoMoreImages) break;
        if (paused == true || 
            fetch.first == Acquisition::HadError ) continue;

        DEBUG("Drawing image " << fetch.second);
        draw_image( img );
    }
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
        if ( imageFile ) {
            DEBUG("Getting current image display status");
            std::auto_ptr<dStorm::Display::Change> c
                = handle->get_state();
            DEBUG("Got status");
            if ( c.get() == NULL )
                throw std::runtime_error("Unable to acquire image from display window");
            DEBUG("Saving image");
            dStorm::Display::Manager::getSingleton()
                .store_image( imageFile(), *c );
            DEBUG("Saved image");
        } else {
            simparm::Message m( "Unable to save image",
                                "No filename for camera snapshot image provided" );
            send(m);
        }
    } else if (&e.source == &stopAim.value && stopAim.triggered()) {
        stopAim.untrigger();
        config.delete_active_selector();
    }
}

void Display::notice_user_key_limits(int key_index, bool lower, std::string input)
{
    ost::MutexLock lock( mutex );
    simparm::optional< boost::units::quantity<cs_units::camera::intensity> > v;
    if ( input != "" )
        v = atof(input.c_str()) * cs_units::camera::ad_count;
    if ( lower )
        lower_user_limit = v;
    else
        upper_user_limit = v;
    redeclare_key = true;
}

Config::Config(const AndorCamera::CameraReference& cam, Context::ConstPtr r)
: simparm::Object("SelectImage", "Select image region"),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  cam(cam),
  context(*r),
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
  context(c.context),
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
    active_selector.reset( new Display( cam, mode, *this, context ) );
    this->simparm::Node::push_back( *active_selector );

    set_entry_viewability();
    active_selector_changed.signal();
}

void Config::delete_active_selector() 
{
    DEBUG("Acquiring selector mutex");
    ost::MutexLock lock( active_selector_mutex );
    DEBUG("Deleting active selector");
    active_selector.reset( NULL );

    DEBUG("Setting entry viewability");
    set_entry_viewability();
    DEBUG("Signalling change");
    active_selector_changed.signal();
    DEBUG("Finished deletion");
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

void Config::context_changed( Context::ConstPtr new_context )
{
    context = *new_context;
    ost::MutexLock lock(active_selector_mutex);
    if ( active_selector.get() ) {
        active_selector->context_changed();
    }
}

}
}
