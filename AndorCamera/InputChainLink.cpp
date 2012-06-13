#include "debug.h"

#include "CameraConnection.h"
#include "InputChainLink.h"
#include "AndorDirect.h"
#include "ViewportSelector.h"
#include <dStorm/UnitEntries/FrameEntry.h>
#include <dStorm/UnitEntries/TimeEntry.h>
#include "ViewportSelector.h"

#include <dStorm/input/Choice.h>
#include <dStorm/input/Link.h>
#include "LiveView.h"
#include <dStorm/input/MetaInfo.h>
#include <dStorm/input/InputMutex.h>
#include <boost/optional.hpp>
#include <dStorm/signals/ResolutionChange.h>
#include <dStorm/signals/BasenameChange.h>

namespace dStorm {
namespace AndorCamera {

using namespace boost::units;

Method::Method()  
: Terminus(),
  name_object("AndorDirectConfig", "Direct camera control"),
  select_ROI("AimCamera","Select ROI"),
  view_ROI("ViewCamera","View ROI only"),
  show_live_by_default("ShowLiveByDefault",
                       "Show camera images live by default",
                       true)
{
    DEBUG("Making AndorDirect config " << this);

    show_live_by_default.set_user_level( simparm::Expert );
}

Method::Method(const Method &c) 
: Terminus(c), 
  name_object(c.name_object),
  select_ROI(c.select_ROI),
  view_ROI(c.view_ROI),
  resolution(c.resolution), basename(c.basename),
  show_live_by_default( c.show_live_by_default )
{
    DEBUG( this << " copied resolution " << resolution[0].is_initialized() << " from " << &c );
    DEBUG("Copied AndorDirect Config");
}

Method::~Method() {
}

void Method::registerNamedEntries( simparm::NodeHandle at ) {
    listening[0] = select_ROI.value.notify_on_value_change( 
        boost::bind( &Method::select_roi_triggered, this ) );
    listening[1] = view_ROI.value.notify_on_value_change( 
        boost::bind( &Method::view_roi_triggered, this ) );

    current_ui = name_object.attach_ui( at );
    select_ROI.attach_ui( current_ui );
    view_ROI.attach_ui( current_ui );
    show_live_by_default.attach_ui( current_ui );
}

void Method::basename_changed( const dStorm::output::Basename& bn ) {
    dStorm::output::Basename b = bn;
    b.set_variable("run", "snapshot");
    try {
        basename = b.new_basename();
        if ( active_selector.get() ) {
            active_selector->basename_changed( basename );
        }
    } catch ( std::runtime_error& ) {
        /* The basename might be nonevaluable due to the presence of 
         * runtime-expanded variables. */
    }
}

dStorm::input::BaseSource* Method::makeSource()
{
    DEBUG(this << " giving resolution " << resolution[0].is_initialized() << " to source");
    std::auto_ptr<CameraConnection> srccon(new CameraConnection("localhost", 0, "52377"));
    std::auto_ptr<CamSource> cam_source( new Source(srccon, show_live_by_default(), resolution ) );
    return cam_source.release();
}

void Method::publish_meta_info() {
    boost::shared_ptr< CamTraits > traits( new CamTraits() );
    traits->push_back( engine::InputPlane() );
    traits->image_number().range().first = 0 * camera::frame;

    dStorm::input::MetaInfo::Ptr mi
        ( new dStorm::input::MetaInfo() );
    mi->set_traits( traits );
    resolution_listener.reset( new boost::signals2::scoped_connection(
        mi->get_signal< signals::ResolutionChange >().connect(
            boost::bind( &Method::resolution_changed, boost::ref(*this), _1 ) )
        ) 
    );
    basename_listener.reset( new boost::signals2::scoped_connection(
        mi->get_signal< signals::BasenameChange >().connect(
            boost::bind( &Method::basename_changed, boost::ref(*this), _1 ) )
        ) 
    );
    update_current_meta_info( mi );
}

void Method::set_display( std::auto_ptr< Display > d ) 
{
    boost::lock_guard<boost::mutex> lock( active_selector_mutex );
    active_selector = d;
    if ( active_selector.get() ) {
        active_selector->resolution_changed( resolution );
        active_selector->basename_changed( basename );
        active_selector->attach_ui( current_ui );
    }

    select_ROI.set_visibility (active_selector.get() == NULL);
    view_ROI.set_visibility (active_selector.get() == NULL);
    active_selector_changed.notify_all();
}

void Method::select_roi_triggered() {
    if ( select_ROI.triggered() ) {
        select_ROI.untrigger();
        std::auto_ptr<CameraConnection> con( new CameraConnection("localhost", 0, "52377") );
        set_display( std::auto_ptr<Display>(new Display( con, Display::SelectROI, *this ) ) );
    }
}
void Method::view_roi_triggered() {
    if ( view_ROI.triggered() ) {
        view_ROI.untrigger();
        std::auto_ptr<CameraConnection> con( new CameraConnection("localhost", 0, "52377") );
        set_display( std::auto_ptr<Display>(new Display( con, Display::ViewROI, *this ) ) );
    }
}

void Method::resolution_changed( const image::MetaInfo<2>::Resolutions& resolution )
{
    DEBUG( this << " setting resolution " << resolution[0].is_initialized() );
    this->resolution = resolution;
    if ( active_selector.get() )
        active_selector->resolution_changed( resolution );
}

std::auto_ptr< dStorm::input::Link > 
get_method() {
    DEBUG("Creating method instance");
    return std::auto_ptr< dStorm::input::Link >(new Method());
}

}
}
