#include "InputChainLink.h"
#include "AndorDirect.h"
#include <dStorm/UnitEntries/FrameEntry.h>
#include <dStorm/UnitEntries/TimeEntry.h>
#include "ViewportSelector.h"
#include <simparm/ChoiceEntry_Impl.hh>

#include <AndorCamera/Camera.h>
#include <AndorCamera/Readout.h>
#include <AndorCamera/Initialization.h>
#include <AndorCamera/Temperature.h>
#include <AndorCamera/Gain.h>
#include <AndorCamera/AcquisitionMode.h>
#include <AndorCamera/Triggering.h>
#include <AndorCamera/ShiftSpeedControl.h>
#include <AndorCamera/ShutterControl.h>

#include <dStorm/input/chain/Choice.h>
#include <dStorm/input/chain/Singleton.h>
#include <dStorm/input/chain/Forwarder.h>
#include "Context.h"
#include "LiveView.h"
#include <dStorm/input/chain/MetaInfo.h>

#include "debug.h"

namespace AndorCamera {

class CameraLink;

struct Method::CameraSwitcher 
: public dStorm::input::chain::Singleton
{
    boost::ptr_list<CameraLink> links;
    dStorm::input::chain::Choice cameras;
  public:
    CameraSwitcher();
};

/** The CameraLink class represents a link to the given camera.
 *  It is a tabbed config set and registered all camera information
 *  sets in the right tabs. */
class CameraLink 
: boost::noncopyable, public simparm::Object, 
  public dStorm::input::chain::Terminus,
  public simparm::Listener
{
    simparm::Set camControl;
    simparm::Set a;
    CameraReference cam;
    simparm::Set b, c, d;

    /** Shorthand for kinetic series length */
    dStorm::IntFrameEntry acquisitionLength;
    /** Short hand for kinetic speed */
    dStorm::FloatTimeEntry acquisitionSpeed;
    ViewportSelector::Config viewportConfig;

    Context::Ptr context;

    void operator()(const simparm::Event&);
    void publish_meta_info();

  public:
    CameraLink( CameraReference camera );
    ~CameraLink(); 
    virtual CameraLink* clone() const 
        { throw std::logic_error("CameraLink unclonable"); }

    virtual void context_changed( ContextRef context, Link* link );
    virtual dStorm::input::BaseSource* makeSource();
    virtual simparm::Node& getNode() { return *this; }

    const CameraReference& getCam() { return cam; }
};

Method::Method()  
: Forwarder(),
  simparm::Object("AndorDirectConfig", "Direct camera control"),
  switcher( new CameraSwitcher() ),
  show_live_by_default("ShowLiveByDefault",
                       "Show camera images live by default",
                       true)
{
    DEBUG("Making AndorDirect config");

    Forwarder::set_more_specialized_link_element( switcher.get() );

    show_live_by_default.userLevel = Object::Expert;
    
    DEBUG("Receiving attach event");
    registerNamedEntries();

    DEBUG("Made AndorDirect config");
}

Method::CameraSwitcher::CameraSwitcher()
: cameras("ChooseCamera", "Choose camera to connect to") 
{
    cameras.getNode().make_thread_safe();
    set_more_specialized( &cameras );

    System& system = System::singleton();
    for (int i = 0; i < system.get_number_of_cameras(); ++i) {
        system.selectCamera(i);
        std::auto_ptr<CameraLink> link( new CameraLink( system.get_current_camera() ) );
        cameras.push_back_choice(*link);
        links.push_back(link);
    }
}

Method::Method(const Method &c) 
: dStorm::input::chain::Forwarder(), simparm::Object(c),
  switcher( c.switcher ),
  show_live_by_default( c.show_live_by_default )
{
    Forwarder::set_more_specialized_link_element( switcher.get() );

    registerNamedEntries();
    DEBUG("Copied AndorDirect Config");
}

Method::~Method() {
    clearChildren();
}

void Method::registerNamedEntries()
{
    //CamTraits::Resolution res;
    //res = resolution_element.get_resolution();
    //dStorm::output::Basename bn( basename() );
    //bn.set_variable( "run", "snapshot" );
    //switcher.reset( new Method::CameraSwitcher(*this, res, bn.new_basename()) );
    //AndorCamera::System& s = AndorCamera::System::singleton();
    //if ( s.get_number_of_cameras() != 0 ) {
        //viewable = true;
        //s.add_listener( *switcher );
        //DEBUG("Selecting camera 0");
        //s.selectCamera(0);
        //DEBUG("Selected camera 0");
    //} else 
        //viewable = false;

    //receive_changes_from( basename.value );
    //receive_changes_from( resolution_element.pixel_size_in_nm );
    //receive_changes_from( resolution_element.pixel_size_in_nm.value );

    push_back( switcher->getNode() );
    push_back( show_live_by_default );
}

void Method::context_changed( ContextRef initial_context, Link* link ) 
{
    last_context.reset( 
        new Context(*initial_context, show_live_by_default())
    );
    Forwarder::context_changed( last_context, link );
}

CameraLink::CameraLink( CameraReference camera ) 
: simparm::Object(camera->getName(), camera->getDesc()),
  simparm::Listener( simparm::Event::ValueChanged ),
  camControl("CamControl", "Camera Control"),
    a("Initialization", "Temperature"),
    cam(camera),
    b("Timings", "Timings"),
    c("AD", "Readout"),
    d("Readout", "Acquisition area"),
    acquisitionLength("AcquisitionLength", "Number of images to acquire"),
    acquisitionSpeed("AcquisitionSpeed", "Exposure time per image"),
    viewportConfig(cam, Context::Ptr( new Context() ) )
{
    simparm::Node::make_thread_safe();
    ost::MutexLock lock( cam->mutex );
    camControl.showTabbed = true;

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

    push_back( acquisitionLength );
    push_back( acquisitionSpeed );
    push_back( camControl );

    receive_changes_from( cam->acquisitionMode().kinetic_length.value );
}

CameraLink::~CameraLink()
{
    ost::MutexLock lock( cam->mutex );
    acquisitionLength.erase( 
        cam->acquisitionMode().kinetic_length.value );
    acquisitionLength.erase( 
        cam->acquisitionMode().kinetic_length["optional_given"] );
    acquisitionSpeed.erase( 
        cam->acquisitionMode().desired_kinetic_cycle_time.value );

    /* Clear sets now to avoid race conditions */
    a.clearChildren();
    b.clearChildren();
    c.clearChildren();
    d.clearChildren();
}

void CameraLink::context_changed( ContextRef c, Link *link )
{
    dStorm::input::chain::Terminus::context_changed(c, link);
    bool publish = (context.get() == NULL);
    context =
        boost::dynamic_pointer_cast<Context,dStorm::input::chain::Context>(c);

    viewportConfig.context_changed( context );

    if ( publish ) 
        publish_meta_info();
}

dStorm::input::BaseSource* CameraLink::makeSource()
{
    const AndorCamera::ImageReadout* im_readout
        = dynamic_cast<const ImageReadout*>( &cam->readout() );

    if ( im_readout == NULL )
        throw std::runtime_error("Readout mode must be image for "
                                 "AndorDirect source");
    if ( ! context->pixel_size.is_set() )
        throw std::runtime_error("Pixel size must be set for live view");

    boost::units::quantity<cs_units::camera::frame_rate> cycle_time
        = cs_units::camera::frame / cam->config().cycleTime();
    boost::shared_ptr<LiveView> live_view( 
        new LiveView( context->default_to_live_view, *context->pixel_size,
                      cycle_time ) );
    std::auto_ptr<CamSource> cam_source( new Source( live_view, cam ) );
    return cam_source.release();
}

void Method::operator()( const simparm::Event& ) {
    if ( last_context.get() && 
         show_live_by_default() != last_context->default_to_live_view ) 
        {
        last_context->default_to_live_view = show_live_by_default();
        notify_of_context_change( last_context );
    }
}

void CameraLink::operator()(const simparm::Event&) {
    publish_meta_info();
}
void CameraLink::publish_meta_info() {
    dStorm::input::Traits<CamImage> traits;
    traits.first_frame = 0;
    traits.last_frame.mark_future_setting(
        cam->acquisitionMode().kinetic_length().is_set()
    );
    traits.speed.mark_future_setting( true );

    TraitsRef mi( new dStorm::input::chain::MetaInfo() );
    mi->traits.reset( traits.clone() );
    notify_of_trait_change( mi );
}

}
