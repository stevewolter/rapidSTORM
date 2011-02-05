#define CIMGBUFFER_ANDORCAMERA_VIEWPORTSELECTOR_CPP
#include "debug.h"

#include "ViewportSelector.h"
#include "CameraConnection.h"
#include "InputChainLink.h"
#include <limits>
#include <boost/units/io.hpp>
#include <boost/variant/apply_visitor.hpp>

#include <dStorm/Image_impl.h>
#include <dStorm/helpers/exception.h>
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/Context_impl.h>
#include <simparm/ChoiceEntry_Impl.hh>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace simparm;
using namespace dStorm::input;
using namespace boost::units;

using dStorm::AndorCamera::CameraPixel;
using dStorm::Pixel;

#define CHECK(x) checkAndorCode( x, __LINE__ )

std::string borderNames[] = { "Left", "Right", "Top", "Bottom" };

namespace dStorm {
namespace AndorCamera {

/** Color depth in the viewport selection image. */
static const int imageDepth = 256;

Display::Display( 
    std::auto_ptr<CameraConnection> cam, 
    Mode mode,
    Method& config
)
: simparm::Set("ViewportSelector", "Viewport settings"),
  ost::Thread("Viewport Selector"),
  cam(cam),
  config(config),
  status("CameraStatus", "Camera status"),
  stopAim("StopAimCamera","Leave aiming mode"),
  pause("PauseCamera", "Pause"),
  imageFile("SaveAcquiredImageFile", "Save camera snapshot to"),
  save("SaveAcquiredImage", "Save camera snapshot"),
  aimed( mode == SelectROI ),
  paused(false),
  change( new dStorm::Display::Change(1) ),
  normalization_factor( 0, 1 ),
  lock_normalization( false ),
  redeclare_key(false)
{
    imageFile.editable = false;
    save.editable = false;
    //imageFile = context.output_basename + ".jpg";

    registerNamedEntries();
    this->ost::Thread::start();
}

void Display::registerNamedEntries() {
    receive_changes_from(stopAim.value);
    receive_changes_from(pause.value);
    receive_changes_from(save.value);

    push_back(status);
    push_back(stopAim);
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
    config.set_display( std::auto_ptr<Display>() );
}

void Display::configure_camera() 
{
    if ( aimed ) {
        /* Set arbitrarily large acquisition area, let andorcamd figure out sensible
         * limits. */
        std::string imro = "in Acquisition in ImageReadout ";
        cam->send(imro + " in TopCaptureBorder in value set 0");
        cam->send(imro + " in LeftCaptureBorder in value set 0");
        cam->send(imro + " in RightCaptureBorder in value set 100000");
        cam->send(imro + " in BottomCaptureBorder in value set 100000");
    }

    /* Acquire with 10 images / second */
    cam->send("in Acquisition in AcquisitionMode in DesiredKineticCycleTime in value set 0.1");
    /* Acquire eternally. */
    cam->send("in Acquisition in AcquisitionMode in SelectAcquisitionMode in value set RunTillAbort");
}

dStorm::Display::ResizeChange Display::getSize() const
{
    dStorm::Display::ResizeChange new_size;
    new_size.size = traits.size;
    new_size.keys.push_back( 
        dStorm::Display::KeyDeclaration("ADC", "A/D counts", imageDepth) );
    new_size.keys.back().can_set_lower_limit = true;
    new_size.keys.back().can_set_upper_limit = true;
    new_size.keys.back().lower_limit = "";
    new_size.keys.back().upper_limit = "";
    if ( context.get() && context->has_info_for<CamImage>() ) {
        const dStorm::input::Traits<CamImage>& t = context->get_info_for<CamImage>();
        for (int i = 0; i < 2; ++i) {
            if ( t.resolution[i].is_set() )
                new_size.pixel_sizes[i] = *t.resolution[i];
        }
    }
        
    return new_size;
}

void Display::initialize_display() 
{

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
         * we have aimed member set, so we have to reset it. */
        if ( aimed ) {
            ost::MutexLock lock(mutex);
            change->do_resize = true;
            change->resize_image = getSize();
            change->do_clear = true;
            change->clear_image.background = 0;
        }
    }

    DEBUG("Initialized display");
        
}

void Display::context_changed( boost::shared_ptr<const input::chain::Context> context ) {
    this->context = context;
    imageFile = context->output_basename + ".jpg";

    change->do_resize = true;
    change->resize_image = getSize();
}

void Display::notice_drawn_rectangle(int l, int r, int t, int b) {
    ost::MutexLock lock(mutex);
    if ( aimed ) {
        /* TODO: Not only for cam 0. */
        std::string prefix = "in Camera0 in Readout in ImageReadout";
        cam->send(prefix + " in TopCaptureBorder in value set " +
            boost::lexical_cast<std::string>(t));
        cam->send(prefix + " in LeftCaptureBorder in value set "  +
            boost::lexical_cast<std::string>(l));
        cam->send(prefix + " in RightCaptureBorder in value set " +
            boost::lexical_cast<std::string>(r));
        cam->send(prefix + " in BottomCaptureBorder in value set " +
            boost::lexical_cast<std::string>(b));

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
            normalization_factor.first = (*lower_user_limit) / camera::ad_count;
        if ( upper_user_limit.is_set() )
            normalization_factor.second = (*upper_user_limit) / camera::ad_count;

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
        bool all_cam_borders_set = true;
        for (int i = 0; i < 4; ++i) all_cam_borders_set = all_cam_borders_set && camBorders[i].is_initialized();
        if ( all_cam_borders_set ) {
            int l = *camBorders[0], r = *camBorders[1],
                t = *camBorders[2], b = *camBorders[3];

            /* Draw the red rectangle that indicates the current acquisition
            * borders */
            for (int v = 0; v < 3; v++) {
                for (int x = l; x <= r; x++)
                    img(x,t) = img(x,b) = Pixel::Red();
                for (int y = t; y <= b; y++)
                    img(l,y) = img(r,y) = Pixel::Red();
        }
        }
#if 0
#endif
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

    struct Display::FetchHandler : public boost::static_visitor<bool> {
        Display& d;
        FetchHandler(Display& d ) : d(d) {}
        bool operator()( const CameraConnection::FetchImage& fe ) {
            CamImage img( d.traits.size, fe.frame_number );
            d.cam->read_data(img);
            d.draw_image(img);
            return true;
        }
        bool operator()( const CameraConnection::ImageError& ) { return true; }
        bool operator()( const CameraConnection::EndOfAcquisition& )
            { return false; }
        bool operator()( const CameraConnection::Simparm& sm ) {
            std::cerr << "ViewportSelector got message '" << sm.message << "'" << std::endl;
            for (int i = 0; i < 4; ++i) {
                std::string indication = "in Camera0 in Readout in ImageReadout in " + borderNames[i] + "CaptureBorder in value set ";
                if ( sm.message.substr(0, indication.length()) == indication )
                    d.camBorders[i] = boost::lexical_cast<int>( sm.message.substr(indication.length()) );
            }
            return true;
        }
    };
void Display::acquire() 
{
    /* Start acquisition with unlimited length. Acquisition is stopped
     * by paused variable. */
    DEBUG("Creating acquisition");
    configure_camera();
    cam->start_acquisition( traits, status );
    DEBUG("Started acquisition");
#if 0
    statusBox.push_back( acq.status );
    statusBox.viewable = true;
#endif

    initialize_display();

    FetchHandler handler(*this);

    while ( ! paused ) {
        CameraConnection::FrameFetch f = cam->next_frame();

        bool keep_going = boost::apply_visitor( handler, f );
        if ( ! keep_going ) break;
    }

    cam->stop_acquisition();
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
        config.set_display( std::auto_ptr<Display>() );
    }
}

void Display::notice_user_key_limits(int key_index, bool lower, std::string input)
{
    ost::MutexLock lock( mutex );
    simparm::optional< boost::units::quantity<camera::intensity> > v;
    if ( input != "" )
        v = atof(input.c_str()) * camera::ad_count;
    if ( lower )
        lower_user_limit = v;
    else
        upper_user_limit = v;
    redeclare_key = true;
}

}
}
