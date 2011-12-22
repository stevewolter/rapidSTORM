#include "debug.h"

#include "CameraConnection.h"
#include "InputChainLink.h"
#include "AndorDirect.h"
#include "ViewportSelector.h"
#include <dStorm/UnitEntries/FrameEntry.h>
#include <dStorm/UnitEntries/TimeEntry.h>
#include "ViewportSelector.h"
#include <simparm/ChoiceEntry_Impl.hh>

#include <dStorm/input/chain/Choice.h>
#include <dStorm/input/chain/Link.h>
#include "LiveView.h"
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/input/InputMutex.h>
#include <boost/optional.hpp>
#include <dStorm/input/ResolutionChange.h>
#include <dStorm/input/BasenameChange.h>

namespace dStorm {
namespace AndorCamera {

using namespace boost::units;

Method::Method()  
: Terminus(),
  simparm::Object("AndorDirectConfig", "Direct camera control"),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  select_ROI("AimCamera","Select ROI"),
  view_ROI("ViewCamera","View ROI only"),
  show_live_by_default("ShowLiveByDefault",
                       "Show camera images live by default",
                       true)
{
    DEBUG("Making AndorDirect config " << this);

    show_live_by_default.userLevel = Object::Expert;
    
    DEBUG("Receiving attach event");
    registerNamedEntries();
}

Method::Method(const Method &c) 
: Terminus(c), simparm::Object(c),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  select_ROI(c.select_ROI),
  view_ROI(c.view_ROI),
  resolution(c.resolution), basename(c.basename),
  show_live_by_default( c.show_live_by_default )
{
    DEBUG( this << " copied resolution " << resolution[0].is_initialized() << " from " << &c );
    registerNamedEntries();
    DEBUG("Copied AndorDirect Config");
}

Method::~Method() {
}

void Method::registerNamedEntries() {
    receive_changes_from(select_ROI.value);
    receive_changes_from(view_ROI.value);

    push_back(select_ROI);
    push_back(view_ROI);
    push_back( show_live_by_default );
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
    boost::shared_ptr< dStorm::input::Traits<engine::Image> > traits;
    traits.reset( new dStorm::input::Traits<engine::Image>() );
    traits->image_number().range().first = 0 * camera::frame;

    dStorm::input::chain::MetaInfo::Ptr mi
        ( new dStorm::input::chain::MetaInfo() );
    mi->set_traits( traits );
    resolution_listener.reset( new boost::signals2::scoped_connection(
        mi->get_signal< input::ResolutionChange >().connect(
            boost::bind( &Method::resolution_changed, boost::ref(*this), _1 ) )
        ) 
    );
    basename_listener.reset( new boost::signals2::scoped_connection(
        mi->get_signal< input::BasenameChange >().connect(
            boost::bind( &Method::basename_changed, boost::ref(*this), _1 ) )
        ) 
    );
    notify_of_trait_change( mi );
}

void Method::set_display( std::auto_ptr< Display > d ) 
{
    boost::lock_guard<boost::mutex> lock( active_selector_mutex );
    active_selector = d;
    if ( active_selector.get() ) {
        active_selector->resolution_changed( resolution );
        active_selector->basename_changed( basename );
        this->simparm::Node::push_back( *active_selector );
    }

    select_ROI.viewable = (active_selector.get() == NULL);
    view_ROI.viewable = (active_selector.get() == NULL);
    active_selector_changed.notify_all();
}

void Method::operator()(const simparm::Event& e)
{
    boost::optional<Display::Mode> mode;
    if ( &e.source == &select_ROI.value && select_ROI.triggered() ) {
        select_ROI.untrigger();
        mode = Display::SelectROI;
    } else if ( &e.source == &view_ROI.value && view_ROI.triggered() ) {
        view_ROI.untrigger();
        mode = Display::ViewROI;
    }

    if ( mode.is_initialized() ) {
        std::auto_ptr<CameraConnection> con( new CameraConnection("localhost", 0, "52377") );
        set_display( std::auto_ptr<Display>(new Display( con, *mode, *this ) ) );
    }
}

void Method::resolution_changed( const dStorm::traits::Optics<2>::Resolutions& resolution )
{
    DEBUG( this << " setting resolution " << resolution[0].is_initialized() );
    this->resolution = resolution;
    if ( active_selector.get() )
        active_selector->resolution_changed( resolution );
}

std::auto_ptr< dStorm::input::chain::Link > 
get_method() {
    DEBUG("Creating method instance");
    return std::auto_ptr< dStorm::input::chain::Link >(new Method());
}

}
}
