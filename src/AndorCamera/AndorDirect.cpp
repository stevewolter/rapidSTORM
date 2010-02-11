#define CImgBuffer_ANDORDIRECT_CPP

#include "AndorDirect.h"
#include <string.h>
#include <sstream>
#include <iomanip>
#include <CImg.h>
#include <foreach.h>
#include <dStorm/input/Buffer.h>
#include <dStorm/input/Source_impl.h>
#include <AndorCamera/Camera.h>
#include <AndorCamera/Readout.h>
#include <AndorCamera/Initialization.h>
#include <AndorCamera/Temperature.h>
#include <AndorCamera/Gain.h>
#include <AndorCamera/AcquisitionMode.h>
#include <AndorCamera/Triggering.h>
#include <AndorCamera/ShiftSpeedControl.h>
#include <AndorCamera/ShutterControl.h>
#include "ViewportSelector.h"

#include "LiveView.h"

#define CHECK(x) checkAndorCode( x, __LINE__ )

using namespace std;
using namespace dStorm::input;
using namespace AndorCamera;
using namespace simparm;

namespace dStorm {
namespace AndorDirect {

CamSource* AndorDirect::Config::impl_makeSource() 
 
{
    CameraReference cam = System::singleton().get_current_camera();

    const AndorCamera::ImageReadout* im_readout
        = dynamic_cast<const ImageReadout*>( &cam->readout() );

    if ( im_readout == NULL )
        throw std::runtime_error("Readout mode must be image for "
                                 "AndorDirect source");

    std::auto_ptr<CamSource> cam_source( new Source( *this, cam ) );
    return cam_source.release();
}

AndorDirect::Source::Source
    (const Config& config, CameraReference& c) 

: Set("AndorDirect", "Direct acquisition"),
  CamSource( static_cast<simparm::Node&>(*this),
    BaseSource::Pushing | BaseSource::Concurrent),
  ost::Thread("Andor camera acquirer"),
  control(c),
  is_initialized( initMutex),
  initialized(false),
  acquisition(control),
  status(acquisition.status),
  live_view( new LiveView(
    config, 
    control->config().cycleTime() *
            cs_units::camera::fps
  ) )
{
    assert( canBePulled() == false );
    assert( pushTarget == NULL ); 

    status.editable = false;

    push_back( c->config() );
    push_back( status );
    push_back( *live_view );
}

AndorDirect::Source::~Source() {
    stopPushing( pushTarget );
    STATUS( "Destructing source" );
}

void Source::run() throw() { 
    try {
        acquire(); 
    } catch (const std::exception& e) {
        std::cerr << "Error in image acquisition: " 
                  << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error in image acquisition." 
                  << std::endl;
    }
}

#define MUST_CONVERT
#define AcquisitionType WORD
#define GetImages GetImages16

void Source::acquire()
{
    long lastValidImage = -1;
    try {
        PROGRESS("Started acquisition subthread");
        acquisition.start();
        PROGRESS("Waiting for acquisition to gain camera");
        acquisition.block_until_on_camera();
        PROGRESS("Acquisition gained camera");
        Traits< CamImage >& my_traits = *this;
        my_traits.size.x() = acquisition.getWidth()
            * cs_units::camera::pixel;
        my_traits.size.y() = acquisition.getHeight()
            * cs_units::camera::pixel;
        my_traits.size.z() = 1 * cs_units::camera::pixel;
        my_traits.dim = 1;
        numImages = acquisition.getLength();
        PROGRESS("Telling " << pushTarget << " number of objects " << 
                 numImages);
        pushTarget->receive_number_of_objects( numImages );

        initMutex.enterMutex();
        initialized = true;
        is_initialized.signal();
        initMutex.leaveMutex();

        /* cancelAcquisition is set in a different thread by stopPushing(). */
        while ( ! cancelAcquisition && acquisition.hasMoreImages() ) {
            auto_ptr<CamImage> image;
            try {
                image.reset(
                    new CamImage(my_traits.size.x().value(), 
                                 my_traits.size.y().value()));
            } catch( const std::bad_alloc& alloc ) {
                /* Do nothing. Try to wait until more memory is available.
                 * Maybe the ring buffer saves us. Maybe not, but we can't
                 * do anything about that without memory. */
                continue;
            }

            Acquisition::Fetch nextIm =
                acquisition.getNextImage( image->ptr() );
            if ( nextIm.first == Acquisition::NoMoreImages )
                break;
            else {
                if ( nextIm.first != Acquisition::HadError )
                    live_view->show( *image, nextIm.second );

                Management policy;
                policy = pushTarget->accept(nextIm.second, 1, image.get());
                if ( policy == Keeps_objects )
                    image.release();
                lastValidImage++;
            }
        }
    } catch (const std::exception& e) {
        cerr << PACKAGE_NAME << ": Image acquisition error: " 
             << e.what() << endl;
    }
    if ( cancelAcquisition ) {
        acquisition.stop();
    } else {
        lastValidImage++;
        pushTarget->accept( lastValidImage, numImages-lastValidImage,
                            NULL );
        lastValidImage = numImages;
    }
}

void Source::waitForInitialization() const {
    PROGRESS("Trying to get initialization wait mutex");
    ost::MutexLock lock(const_cast<ost::Mutex&>(initMutex));
    while ( !initialized ) {
        PROGRESS("Waiting for acquisition initialization");
        const_cast<ost::Condition&>(is_initialized).wait();
        PROGRESS("Waited for acquisition initialization");
    }
}

int Source::quantity() const { 
    return numImages;
}

void Source::startPushing(Drain<CamImage> *target) 
 
{
    PROGRESS("AndorDirect is told to start pushing images");
    if (this->pushTarget != NULL)
        throw runtime_error
            ("This AndorDirect source already has a target.");

    pushTarget = target;
    cancelAcquisition = false;

    PROGRESS("Starting acquisition subthread");
    Thread::start();

    waitForInitialization();
}

void Source::stopPushing(Drain<CamImage> *target) {
    if ( pushTarget == NULL ) return;

    if (pushTarget != target)
        throw std::logic_error(
                    "This AndorDirect source does not push to the "
                    "specified target.");
    cancelAcquisition = true;
    Thread::join();
    pushTarget = NULL;
}

Config::Config(input::Config& config)  
: CamConfig("AndorDirectConfig", "Direct camera control"),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  basename("OutputBasename", "Base name for output files",
           "dStorm_acquisition_$run$"),
  show_live_by_default("ShowLiveByDefault",
                       "Show camera images live by default",
                       true),
  live_show_frequency("LiveShowFrequency",
            "Show live camera images every x seconds",
            0.1),
  resolution_element( config.pixel_size_in_nm )
{
    output_file_basename = basename();

    PROGRESS("Making AndorDirect config");

    show_live_by_default.userLevel = Object::Expert;
    live_show_frequency.userLevel = Object::Expert;
    push_back( config.pixel_size_in_nm );
    
    LOCKING("Receiving attach event");
    registerNamedEntries();

    PROGRESS("Made AndorDirect config");
}

Config::Config(const Config &c, input::Config& config) 
: CamConfig(c),
  Node::Callback( simparm::Event::ValueChanged ),
  basename(c.basename),
  show_live_by_default( c.show_live_by_default ),
  live_show_frequency( c.live_show_frequency ),
  resolution_element( config.pixel_size_in_nm )
{
    push_back( config.pixel_size_in_nm );

    registerNamedEntries();
}

Config::~Config() {
    switcher.reset(NULL);
}

/** The CameraLink class represents a link to the given camera.
 *  It is a tabbed config set and registered all camera information
 *  sets in the right tabs. */
class CameraLink : public simparm::Set {
    simparm::Set a;
    CameraReference cam;
    simparm::Set b, c, d;

    /** Shorthand for kinetic series length */
    simparm::UnsignedLongEntry acquisitionLength;
    /** Short hand for kinetic speed */
    simparm::DoubleEntry acquisitionSpeed;
    ViewportSelector::Config viewportConfig;

  public:
    CameraLink( simparm::Node& node, CameraReference camera,
                CamTraits::Resolution resolution )
    : simparm::Set("CamControl", "Camera Control"),
      a("Initialization", "Temperature"),
      cam(camera),
      b("Timings", "Timings"),
      c("AD", "Readout"),
      d("Readout", "Acquisition area"),
      acquisitionLength("AcquisitionLength", "Number of images to acquire"),
      acquisitionSpeed("AcquisitionSpeed", "Exposure time per image"),
      viewportConfig(cam, resolution)
    {
        showTabbed = true;

        a.push_back( cam->initialization() );
        a.push_back( cam->temperature() );
        a.push_back( cam->triggering() );
        a.push_back( cam->shutter_control() );

        b.push_back( cam->acquisitionMode() );

        c.push_back( cam->shift_speed_control() );
        c.push_back( cam->gain() );

        d.push_back( cam->readout() );
        d.push_back( viewportConfig );

        push_back( a );
        push_back( b );
        push_back( c );
        push_back( d );

        /* Insert the camera's value elements into the 
         * acquisitionLength and acquisitionSpeed entries
         * instead of their own. This allows a different
         * description, but same values. */
        acquisitionLength.erase( acquisitionLength.value );
        acquisitionLength.push_back( 
            cam->acquisitionMode().kinetic_length.value );
        acquisitionSpeed.erase( acquisitionSpeed.value );
        acquisitionSpeed.push_back( 
            cam->acquisitionMode().desired_kinetic_cycle_time.value );

        node.push_back( acquisitionLength );
        node.push_back( acquisitionSpeed );
        node.push_back( *this );
    }
    ~CameraLink() {
        ost::MutexLock lock( cam->mutex );
        acquisitionLength.erase( 
            cam->acquisitionMode().kinetic_length.value );
        acquisitionSpeed.erase( 
            cam->acquisitionMode().desired_kinetic_cycle_time.value );
    }

    const CameraReference& getCam() { return cam; }
    void change_resolution( CamTraits::Resolution resolution )
        { viewportConfig.set_resolution( resolution ); }
};

class Config::CameraSwitcher : public AndorCamera::System::Listener 
{
    simparm::Node& node;
    CamTraits::Resolution resolution;

    std::auto_ptr<CameraLink> currentlyActive;
  public:
    CameraSwitcher(simparm::Node& node, CamTraits::Resolution r)
        : node(node), resolution(r) {}
    ~CameraSwitcher() {
        /* Prevent destruction cycles. If we receive camera change events
         * during destruction, strange things might happen. */
        AndorCamera::System::singleton().remove_listener( *this );
        current_camera_changed(-1,-1);
        currentlyActive.reset(NULL);
    }

    void current_camera_changed(int, int to) {
        STATUS("Changing current camera to " << to);
        if ( to != -1 ) 
        {
            CameraReference cam = AndorCamera::System::singleton()
                                               .get_current_camera();
            ost::MutexLock lock( cam->mutex );
            currentlyActive.reset( 
                new CameraLink( node, cam, resolution ) );
        } else {
            currentlyActive.reset( NULL );
        }
    }

    void change_resolution( CamTraits::Resolution r ) {
        resolution = r;
        if (currentlyActive.get())
            currentlyActive->change_resolution( r );
    }
};

void Config::registerNamedEntries()
{
    CamTraits::Resolution res;
    res.fill( cs_units::camera::pixels_per_meter * 1E9f
              / float(resolution_element()) );
    switcher.reset( new Config::CameraSwitcher(*this, res) );
    AndorCamera::System& s = AndorCamera::System::singleton();
    if ( s.get_number_of_cameras() != 0 ) {
        viewable = true;
        s.add_listener( *switcher );
        s.selectCamera(0);
    } else 
        viewable = false;

    receive_changes_from( basename.value );

    push_back( basename );
    push_back( show_live_by_default );
    push_back( live_show_frequency );
}

void Config::operator()(const simparm::Event& e)
{
    if ( &e.source == &resolution_element ) {
        if ( switcher.get() != NULL )
            switcher->change_resolution( 
                CamTraits::Resolution( resolution_element() * 1E-9 ) );
    } else if ( &e.source == &basename.value ) {
        output_file_basename = basename();
    }
}

}
}
