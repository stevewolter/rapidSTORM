#define CImgBuffer_ANDORDIRECT_CPP

#include "debug.h"

#include "AndorDirect.h"
#include <string.h>
#include <sstream>
#include <iomanip>
#include <dStorm/input/Buffer.h>
#include <dStorm/output/Basename.h>
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
using namespace dStorm;
using namespace simparm;

namespace AndorCamera {

CamSource* Method::impl_makeSource() 
 
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

Source::Source
    (const Method& config, CameraReference& c) 

: Set("AndorDirect", "Direct acquisition"),
  CamSource( static_cast<simparm::Node&>(*this),
    BaseSource::Flags().set(BaseSource::TimeCritical) ),
  control(c),
  is_initialized( initMutex),
  initialized(false),
  error_in_initialization(false),
  acquisition(control),
  status(acquisition.status),
  live_view( new LiveView(
    config, 
    cs_units::camera::frame / control->config().cycleTime()
  ) )
{
    status.editable = false;

    push_back( c->config() );
    push_back( status );
    push_back( *live_view );
    DEBUG("Built AndorDirect source");
}

Source::~Source() {
    DEBUG( "Destructing source" );
}

#define MUST_CONVERT
#define AcquisitionType WORD
#define GetImages GetImages16

void Source::waitForInitialization() const {
    DEBUG("Trying to get initialization wait mutex");
    ost::MutexLock lock(initMutex);
    while ( !initialized ) {
        DEBUG("Waiting for acquisition initialization");
        is_initialized.wait();
        DEBUG("Waited for acquisition initialization");
    }
    if ( error_in_initialization )
        throw std::runtime_error(
            "An error in image acquisition prevents running a job");
}

Method::Method(input::Config& config)  
: CamConfig("AndorDirectConfig", "Direct camera control"),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  basename("OutputBasename", "Base name for output files",
           "dStorm_acquisition_$run$"),
  show_live_by_default("ShowLiveByDefault",
                       "Show camera images live by default",
                       true),
  resolution_element( config )
{
    output_file_basename = basename();

    DEBUG("Making AndorDirect config");

    show_live_by_default.userLevel = Object::Expert;
    push_back( config.pixel_size_in_nm );
    
    DEBUG("Receiving attach event");
    registerNamedEntries();

    DEBUG("Made AndorDirect config");
}

Method::Method(const Method &c, input::Config& config) 
: CamConfig(c),
  Node::Callback( simparm::Event::ValueChanged ),
  basename(c.basename),
  show_live_by_default( c.show_live_by_default ),
  resolution_element( config )
{
    DEBUG("Copying AndorDirect Config");
    push_back( config.pixel_size_in_nm );

    registerNamedEntries();
    DEBUG("Copied AndorDirect Config");
}

Method::~Method() {
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
    dStorm::IntFrameEntry acquisitionLength;
    /** Short hand for kinetic speed */
    dStorm::FloatTimeEntry acquisitionSpeed;
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
        ost::MutexLock lock( cam->mutex );
        showTabbed = true;

        a.push_back( cam->initialization() );
        a.push_back( cam->temperature() );
        a.push_back( cam->triggering() );
        a.push_back( cam->shutter_control() );
        a.push_back( cam->state_machine().state );

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
        acquisitionLength.push_back( 
            cam->acquisitionMode().kinetic_length["optional_given"] );
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

        /* Clear sets now to avoid race conditions */
        a.clearChildren();
        b.clearChildren();
        c.clearChildren();
        d.clearChildren();
    }

    const CameraReference& getCam() { return cam; }
    void change_resolution( CamTraits::Resolution resolution )
        { viewportConfig.set_resolution( resolution ); }
    void change_basename( std::string basename ) { 
        DEBUG("Setting output file basename to " << basename);
        viewportConfig.set_output_file( basename + ".tif" ); 
    }
};

class Method::CameraSwitcher : public AndorCamera::System::Listener 
{
    simparm::Node& node;
    CamTraits::Resolution resolution;
    std::string basename;

    std::auto_ptr<CameraLink> currentlyActive;
  public:
    CameraSwitcher(simparm::Node& node, CamTraits::Resolution r, std::string basename)
        : node(node), resolution(r), basename(basename) {}
    ~CameraSwitcher() {
        /* Prevent destruction cycles. If we receive camera change events
         * during destruction, strange things might happen. */
        AndorCamera::System::singleton().remove_listener( *this );
        current_camera_changed(-1,-1);
        currentlyActive.reset(NULL);
    }

    void current_camera_changed(int, int to) {
        DEBUG("Changing current camera to " << to);
        if ( to != -1 ) 
        {
            CameraReference cam = AndorCamera::System::singleton()
                                               .get_current_camera();
            currentlyActive.reset( 
                new CameraLink( node, cam, resolution ) );
            currentlyActive->change_basename( basename );
        } else {
            currentlyActive.reset( NULL );
        }
    }

    void change_resolution( CamTraits::Resolution r ) {
        resolution = r;
        if (currentlyActive.get())
            currentlyActive->change_resolution( r );
    }

    void change_basename( std::string name ) {
        basename = name;
        if (currentlyActive.get())
            currentlyActive->change_basename( name );
    }
};

void Method::registerNamedEntries()
{
    CamTraits::Resolution res;
    res = resolution_element.get_resolution();
    dStorm::output::Basename bn( basename() );
    bn.set_variable( "run", "snapshot" );
    switcher.reset( new Method::CameraSwitcher(*this, res, bn.new_basename()) );
    AndorCamera::System& s = AndorCamera::System::singleton();
    if ( s.get_number_of_cameras() != 0 ) {
        viewable = true;
        s.add_listener( *switcher );
        DEBUG("Selecting camera 0");
        s.selectCamera(0);
        DEBUG("Selected camera 0");
    } else 
        viewable = false;

    receive_changes_from( basename.value );
    receive_changes_from( resolution_element.pixel_size_in_nm );

    push_back( basename );
    push_back( show_live_by_default );
}

void Method::operator()(const simparm::Event& e)
{
    if ( &e.source == &resolution_element.pixel_size_in_nm ) {
        if ( switcher.get() != NULL )
            switcher->change_resolution( resolution_element.get_resolution() );
    } else if ( &e.source == &basename.value ) {
        output_file_basename = basename();
        dStorm::output::Basename bn( output_file_basename );
        bn.set_variable( "run", "" );
        switcher->change_basename( bn.new_basename() );
    }
}

Source::TraitsPtr Source::get_traits() 
{
    DEBUG("Waiting for camera initialization to get traits");
    waitForInitialization();
    DEBUG("Got camera initialization");
    TraitsPtr rv( new TraitsPtr::element_type() );
    rv->size.x() = acquisition.getWidth();
    rv->size.y() = acquisition.getHeight();
    DEBUG("Acquisition has a length set: " << acquisition.hasLength());
    if ( acquisition.hasLength() )
        rv->last_frame = acquisition.getLength() - 1 * cs_units::camera::frame;
    return rv;
}

}
