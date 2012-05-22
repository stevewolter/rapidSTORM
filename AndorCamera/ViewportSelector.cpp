#define VERBOSE
#include "debug.h"

#include "CameraConnection.h"
#include "ViewportSelector.h"
#include "InputChainLink.h"
#include <limits>
#include <boost/units/io.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <simparm/Message.hh>

#include <dStorm/Image_impl.h>
#include <dStorm/image/extend.h>
#include <simparm/ChoiceEntry_Impl.hh>
#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/qi.hpp>

using namespace std;
using namespace simparm;
using namespace dStorm::input;
using namespace boost::units;

using dStorm::AndorCamera::CameraPixel;
using dStorm::Pixel;

namespace dStorm {
namespace AndorCamera {

/** Color depth in the viewport selection image. */
static const int imageDepth = 256;

Display::Display( 
    std::auto_ptr<CameraConnection> cam, 
    Mode mode,
    Method& config
)
: cam(cam),
  config(config),
  name_object("ViewportSelector", "Viewport settings"),
  status("CameraStatus", "Camera status"),
  stopAim("StopAimCamera","Leave aiming mode"),
  pause("PauseCamera", "Pause"),
  imageFile("SaveAcquiredImageFile", "Save camera snapshot to"),
  save("SaveAcquiredImage", "Save camera snapshot"),
  aimed( mode == SelectROI ),
  paused(false),
  change( new display::Change(1) ),
  normalization_factor( 0, 1 ),
  lock_normalization( false ),
  redeclare_key(false)
{
    DEBUG("Constructed display " << this);
    imageFile.editable = false;
    save.editable = false;

    image_acquirer = boost::thread( &Display::run, this );
}

void Display::attach_ui(simparm::Node& at) {
    receive_changes_from(stopAim.value);
    receive_changes_from(pause.value);
    receive_changes_from(save.value);

    current_ui = name_object.attach_ui(at);
    simparm::NodeRef r = *current_ui;
    status.attach_ui(r);
    stopAim.attach_ui(r);
    pause.attach_ui(r);
    imageFile.attach_ui(r);
    save.attach_ui(r);
}

Display::~Display() {
    DEBUG("Destructing ViewportSelector " << this);
    paused = true;
    image_acquirer.join();
    DEBUG("Destructed ViewportSelector " << this);
}

std::auto_ptr<display::Change> 
Display::get_changes()
{
    DEBUG("Fetching changes");
    std::auto_ptr<display::Change> other
        ( new display::Change(1) );

    boost::lock_guard<boost::mutex> lock(mutex);
    std::swap( other, this->change );
    DEBUG("Fetched changes");
    return other;
}

void Display::notice_closed_data_window() {
    config.set_display( std::auto_ptr<Display>() );
}

void Display::configure_camera() 
{
    DEBUG("Configuring camera");
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
    DEBUG("Configured camera");
}

display::ResizeChange Display::getSize() const
{
    assert( traits.plane_count() > 0 );
    display::ResizeChange new_size;
    new_size.set_size( ImageTypes<2>::Size(traits.image(0).size) );
    new_size.keys.push_back( 
        display::KeyDeclaration("ADC", "A/D counts", imageDepth) );
    new_size.keys.back().can_set_lower_limit = true;
    new_size.keys.back().can_set_upper_limit = true;
    new_size.keys.back().lower_limit = "";
    new_size.keys.back().upper_limit = "";
    for (int i = 0; i < 2; ++i)
        if ( resolution[i].is_initialized() )
            new_size.pixel_sizes[i] = *resolution[i];
    return new_size;
}

void Display::initialize_display() 
{

    if ( handle.get() == NULL ) {
        display::Manager::WindowProperties props;
        props.name = "Live camera view";
        props.flags.close_window_on_unregister();
        if ( aimed )
            props.flags.notice_drawn_rectangle();
        else
            props.flags.zoom_on_drawn_rectangle();
        props.initial_size = getSize();

        DEBUG("Initializing display handle");
        boost::lock_guard<boost::mutex> lock(mutex);
        DEBUG("Got lock for display handle");
        handle = display::Manager::getSingleton()
            .register_data_source( props, *this );
        change->do_resize = false;
        change->do_clear = true;
        change->clear_image.background = 0;
    } else {
        /* Window already open. The size, however, might have changed if
         * we have aimed member set, so we have to reset it. */
        if ( aimed ) {
            boost::lock_guard<boost::mutex> lock(mutex);
            if ( traits.plane_count() > 0 ) {
                change->do_resize = true;
                change->resize_image = getSize();
                change->do_clear = true;
                change->clear_image.background = 0;
            }
        }
    }

    DEBUG("Initialized display");
        
}

void Display::resolution_changed( image::MetaInfo<2>::Resolutions r )
{
    this->resolution = r;
    if ( traits.plane_count() > 0 ) {
        change->do_resize = true;
        change->resize_image = getSize();
    }
}

void Display::basename_changed( const std::string& basename ) {
    imageFile = basename + ".jpg";
}

void Display::notice_drawn_rectangle(int l, int r, int t, int b) {
    boost::lock_guard<boost::mutex> lock(mutex);
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
        change->image_change.new_image = extend( last_image, dStorm::display::Image() );
    }
}

void Display::draw_image( const CamImage& data) {
    boost::lock_guard<boost::mutex> lock(mutex);

    /* Compute normalization and new key. */
    if ( ! lock_normalization ) {
        if ( ! lower_user_limit.is_initialized() || ! upper_user_limit.is_initialized() ) {
            normalization_factor = data.minmax();
            redeclare_key = true;
        }
    }
    
    if ( redeclare_key ) {
        if ( lower_user_limit.is_initialized() )
            normalization_factor.first = (*lower_user_limit) / camera::ad_count;
        if ( upper_user_limit.is_initialized() )
            normalization_factor.second = (*upper_user_limit) / camera::ad_count;

        change->changed_keys.front().clear();
        change->changed_keys.front().reserve( imageDepth );
        for (int i = 0; i < imageDepth; i++)
            change->changed_keys.front().push_back( display::KeyChange(
                i, dStorm::Pixel(i),
                i * 1.0 * (normalization_factor.second - normalization_factor.first)
                    / imageDepth + normalization_factor.first ));
        redeclare_key = false;
    }
    /* Normalize pixels and store result in the ImageChange vector */
    dStorm::display::Image& img = change->image_change.new_image;
    img = extend( data.normalize<dStorm::Pixel>(normalization_factor), dStorm::display::Image() );
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
                    img(x,t,0) = img(x,b,0) = Pixel::Red();
                for (int y = t; y <= b; y++)
                    img(l,y,0) = img(r,y,0) = Pixel::Red();
        }
        }
    }
    change->do_change_image = true;

    last_image = img;
}

void Display::run() throw() {
    try {
        acquire(); 
        return;
    } catch (const std::exception& e) {
        simparm::Message m( "Could not acquire images for aiming view", e.what() );
        current_ui->send(m);
    } catch (...) {
        simparm::Message m( "Could not acquire images for aiming view", 
                            "Unknown error occured while acquiring images for aiming view." );
        current_ui->send(m);
    }
    handle.reset( NULL );
    DEBUG("Display acquisition thread finished\n");
}

    struct Display::FetchHandler : public boost::static_visitor<bool> {
        Display& d;
        boost::spirit::qi::symbols<char, int> border_names;

        FetchHandler(Display& d ) : d(d) {
            border_names.add("Left", 0)("Right", 1)("Top", 2)("Bottom", 3); };
        bool operator()( const CameraConnection::FetchImage& fe ) {
            assert( d.traits.plane_count() > 0 );
            CamImage img( d.traits.image(0).size, fe.frame_number );
            d.cam->read_data(img);
            d.draw_image(img);
            return true;
        }
        bool operator()( const CameraConnection::ImageError& ) { return true; }
        bool operator()( const CameraConnection::EndOfAcquisition& )
            { return false; }
        bool operator()( const CameraConnection::Simparm& sm ) {
            namespace qi = boost::spirit::qi;
            namespace ascii = boost::spirit::ascii;
            namespace fsn = boost::fusion;

            std::string::const_iterator begin = sm.message.begin(), end = sm.message.end();
            fsn::vector2<int,int> assignment;
            bool success = phrase_parse( begin, end,
                "in Camera0 in Readout in ImageReadout in " >> border_names >> "CaptureBorder in value set " >> qi::int_,
                ascii::space, assignment );
            if ( success && begin == end )
                d.camBorders[fsn::at_c<0>(assignment)] = fsn::at_c<1>(assignment);
            return true;
        }
        bool operator()( const CameraConnection::StatusChange& s ) {
            d.status = s.status;
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

    initialize_display();

    FetchHandler handler(*this);

    while ( ! paused ) {
        CameraConnection::FrameFetch f = cam->next_frame();

        bool keep_going = boost::apply_visitor( handler, f );
        if ( ! keep_going ) break;
    }

    cam->stop_acquisition();
}

void Display::operator()(const simparm::Event& e) 
{ 
    DEBUG("Handling events for display " << this);
    if (&e.source == &pause.value && pause.triggered()) {
        pause.untrigger();
        /* No lock  necessary here, since pause is an atomic comparison */
        paused = !paused;
        imageFile.editable = paused;
        save.editable = paused;
        if ( paused )
            image_acquirer.join();
        else
            image_acquirer = boost::thread( &Display::run, this );
    } else if (&e.source == &save.value && save.triggered()) {
        save.untrigger();
        if ( ! imageFile ) {
            simparm::Message m( "Unable to save image",
                                "No filename for camera snapshot image provided" );
            current_ui->send(m);
        } else if ( ! handle.get() ) {
            simparm::Message m( "Unable to save image",
                                "Image display window has already been closed" );
            current_ui->send(m);
        } else {
            DEBUG("Getting current image display status");
            display::SaveRequest request;
            request.filename = imageFile();
            handle->store_current_display( request );
            DEBUG("Saved image");
        }
    } else if (&e.source == &stopAim.value && stopAim.triggered()) {
        stopAim.untrigger();
        config.set_display( std::auto_ptr<Display>() );
    }
}

void Display::notice_user_key_limits(int key_index, bool lower, std::string input)
{
    boost::lock_guard<boost::mutex> lock( mutex );
    boost::optional< boost::units::quantity<camera::intensity> > v;
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
