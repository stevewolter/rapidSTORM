#include "debug.h"

#include "InputChainLink.h"
#include "AndorDirect.h"
#include <dStorm/UnitEntries/FrameEntry.h>
#include <dStorm/UnitEntries/TimeEntry.h>
//#include "ViewportSelector.h"
#include <simparm/ChoiceEntry_Impl.hh>

#include <dStorm/input/chain/Choice.h>
#include <dStorm/input/chain/Singleton.h>
#include <dStorm/input/chain/Link.h>
#include <dStorm/input/chain/Context_impl.h>
#include "LiveView.h"
#include "CameraConnection.h"
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/input/InputMutex.h>

namespace dStorm {
namespace AndorCamera {

using namespace boost::units;

Method::Method()  
: Terminus(),
  simparm::Object("AndorDirectConfig", "Direct camera control"),
  show_live_by_default("ShowLiveByDefault",
                       "Show camera images live by default",
                       true)
{
    DEBUG("Making AndorDirect config");

    show_live_by_default.userLevel = Object::Expert;
    
    DEBUG("Receiving attach event");
    registerNamedEntries();

    DEBUG("Made AndorDirect config");
}

Method::Method(const Method &c) 
: Terminus(c), simparm::Object(c),
  show_live_by_default( c.show_live_by_default )
{
    registerNamedEntries();
    DEBUG("Copied AndorDirect Config");
}

Method::~Method() {
}

void Method::registerNamedEntries() {
    push_back( show_live_by_default );
}

dStorm::input::chain::Link::AtEnd
Method::context_changed( ContextRef initial_context, Link* link ) 
{
    dStorm::input::chain::Link::context_changed(initial_context, link);
    last_context = initial_context;
    if ( ! published.get() )
        return publish_meta_info();
    else
        return AtEnd();
}

dStorm::input::BaseSource* Method::makeSource()
{
    LiveView::Resolution resolution;
    if ( last_context->has_info_for<CamImage>() )
        resolution = last_context->get_info_for<CamImage>().resolution;

#if 0
    quantity<camera::frame_rate> cycle_time
        = camera::frame / cam->config().cycleTime();
    boost::shared_ptr<LiveView> live_view( 
        new LiveView( context->default_to_live_view, resolution, cycle_time ) );
#endif
    std::auto_ptr<CameraConnection> srccon(new CameraConnection("localhost", 0, "52377"));
    std::auto_ptr<CamSource> cam_source( new Source(srccon, show_live_by_default() ) );
    return cam_source.release();
}

input::chain::Link::AtEnd Method::publish_meta_info() {
    if ( last_context.get() == NULL )  {
        DEBUG("CameraLink publishing null traits");
        return notify_of_trait_change( TraitsRef() );
    } else if ( published.get() != NULL ) {
        return notify_of_trait_change( published );
    } else {
        boost::shared_ptr< dStorm::input::Traits<CamImage> > traits;
        if ( last_context->has_info_for<CamImage>() )
            traits.reset( last_context->get_info_for<CamImage>().clone() );
        else
            traits.reset( new dStorm::input::Traits<CamImage>() );
        traits->image_number().range().first = 0 * camera::frame;

        dStorm::input::chain::MetaInfo::Ptr mi
            ( new dStorm::input::chain::MetaInfo() );
        mi->set_traits( traits );
        published = mi;
        return notify_of_trait_change( published );
    }
#if 0
    boost::shared_ptr< dStorm::input::Traits<CamImage> > traits;
    if ( context->has_info_for<CamImage>() )
        traits.reset( context->get_info_for<CamImage>().clone() );
    else
        traits.reset( new dStorm::input::Traits<CamImage>() );

    if ( cam->acquisitionMode().kinetic_length().is_set() )
        traits->image_number().range().second.promise( dStorm::deferred::JobTraits );
    traits->image_number().resolution().promise( dStorm::deferred::JobTraits );

    dStorm::input::chain::MetaInfo::Ptr mi
        ( new dStorm::input::chain::MetaInfo() );
    mi->set_traits( traits );
    DEBUG("CameraLink publishing non-null traits");
    notify_of_trait_change( mi );
#endif
}

}
}
