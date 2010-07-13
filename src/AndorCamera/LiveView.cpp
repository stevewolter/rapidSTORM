#include "debug.h"

#include "LiveView.h"
#include <boost/thread/locks.hpp>
#include "AndorDirect.h"
#include <dStorm/Image.h>
#include <dStorm/Image_impl.h>

typedef ost::MutexLock guard;

using namespace boost::units;
using namespace cs_units::camera;
using namespace dStorm;

namespace AndorCamera {

LiveView::LiveView( 
    const Method& config,
    boost::units::quantity<cs_units::camera::frame_rate> cycle_time
    )
: Object("LiveView", "Live view options"),
  cycle_time( cycle_time ),
  resolution( *config.resolution_element.get_resolution() ),
  show_live("ShowLive", "Show camera image", 
            config.show_live_by_default()),
  change( new dStorm::Display::Change() )
{
    DEBUG("LiveView constructed");
    registerNamedEntries();
}

void LiveView::registerNamedEntries() {
    push_back( show_live );
}

LiveView::~LiveView() {
    DEBUG("LiveView destructing");
}

void LiveView::show_window(CamImage::Size size) {
    if ( window.get() == NULL ) {
        DEBUG("Showing live view window");
        dStorm::Display::Manager::WindowProperties props;
        props.name = "Live camera view";
        props.flags.close_window_on_unregister();
        props.initial_size.size = size;
        props.initial_size.key_size = 256;
        props.initial_size.pixel_size = 
            resolution;

        window = dStorm::Display::Manager::getSingleton()
            .register_data_source( props, *this );
        DEBUG("Success in showing live view window");
    }
}

void LiveView::hide_window() {
    if ( window.get() != NULL ) {
        DEBUG("Hiding window");
        show_live = false;
        window.reset( NULL );
    }
}

std::auto_ptr<Display::Change> LiveView::get_changes()
{
    DEBUG("Locking changes");
    std::auto_ptr<Display::Change> fresh( new Display::Change() );
    ost::MutexLock lock(change_mutex);
    DEBUG("Getting live view changes");
    compute_image_change( current_image_content );
    std::swap( fresh, change );
    DEBUG("Got live view changes");
    return fresh;
}

void LiveView::compute_image_change
    ( const CamImage& image )
{
    DEBUG("Computing minmax for change " << change.get() << " and validity " << image.is_invalid());
    if ( ! image.is_invalid() ) {
        CamImage::PixelPair minmax = image.minmax();
        
        DEBUG("Normalizing");
        change->do_change_image = true;
        change->image_change.new_image = 
            image.normalize<dStorm::Pixel>();
        DEBUG("Making key");
        change->make_linear_key( minmax );
    }
}

void LiveView::show( const CamImage& image, int number) {
    DEBUG("Showing image");
    guard lock(window_mutex);
    DEBUG("Got mutex for showing image");
    if ( show_live() ) {
        DEBUG("Performing show");
        show_window(image.sizes());

        current_image_content = image;
        DEBUG("Performed show");
    } else {
        DEBUG("Hiding window");
        if ( window.get() != NULL ) hide_window();
    }
}

void LiveView::notice_closed_data_window() {
    guard lock(window_mutex);
    hide_window();
}

}
