#include "debug.h"

#include "LiveView.h"
#include <boost/thread/locks.hpp>
#include "AndorDirect.h"
#include <dStorm/Image.h>
#include <dStorm/image/extend.h>
#include <dStorm/Image_impl.h>
#include <simparm/Node.h>

using namespace boost::units;
using namespace camera;
using namespace dStorm;

namespace dStorm {
namespace AndorCamera {

LiveView::LiveView( 
    bool on_by_default,
    Resolution resolution
    )
: resolution( resolution ),
  show_live("ShowLive", "Show camera image", on_by_default),
  change( new display::Change(1) )
{
    DEBUG("LiveView constructed");
}

void LiveView::attach_ui( simparm::NodeHandle at ) {
    show_live.attach_ui( at );
}

LiveView::~LiveView() {
    DEBUG("LiveView destructing");
}

void LiveView::show_window(CamImage::Size size) {
    if ( window.get() == NULL ) {
        DEBUG("Showing live view window");
        assert( size.x() < 10240 * camera::pixel && size.y() < 10240 * camera::pixel);
        display::WindowProperties props;
        props.name = "Live camera view";
        props.flags.close_window_on_unregister();
        props.initial_size.set_size( size );
        props.initial_size.keys.push_back( 
            display::KeyDeclaration("ADC", "A/D counts", 256) );
        props.initial_size.keys.back().can_set_lower_limit = true;
        props.initial_size.keys.back().can_set_upper_limit = true;
        props.initial_size.keys.back().lower_limit = "";
        props.initial_size.keys.back().upper_limit = "";
        for (int i = 0; i < 2; ++i)
            if ( resolution[i].is_initialized() )
                props.initial_size.pixel_sizes[i] = *resolution[i];

        window = show_live.get_user_interface_handle()->get_image_window( props, *this );
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

std::auto_ptr<display::Change> LiveView::get_changes()
{
    DEBUG("Locking changes");
    std::auto_ptr<display::Change> fresh( new display::Change(1) );
    boost::lock_guard<boost::mutex> lock(change_mutex);
    DEBUG("Getting live view changes");
    if ( current_image_content.is_valid() ) {
        compute_image_change( current_image_content );
        current_image_content.invalidate();
    }

    std::swap( fresh, change );
    DEBUG("Got live view changes");
    return fresh;
}

void LiveView::compute_image_change
    ( const CamImage& image )
{
    DEBUG("Computing minmax for change " << change.get() << " and validity " << image.is_invalid());
    if ( ! image.is_invalid() ) {
        CamImage::PixelPair minmax;
        if ( ! lower_user_limit.is_initialized() || ! upper_user_limit.is_initialized() )
            minmax = image.minmax();
        if ( lower_user_limit.is_initialized() )
            minmax.first = (*lower_user_limit) / camera::ad_count;
        if ( upper_user_limit.is_initialized() )
            minmax.second = (*upper_user_limit) / camera::ad_count;
        
        DEBUG("Normalizing");
        change->do_change_image = true;
        change->image_change.new_image = 
            extend( image.normalize<dStorm::Pixel>( minmax ), dStorm::display::Image() );
        DEBUG("Making key");
        change->make_linear_key( minmax );
    }
}

void LiveView::show( const CamImage& image) {
    DEBUG("Showing image");
    boost::lock_guard<boost::mutex> lock(window_mutex);
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
    boost::lock_guard<boost::mutex> lock(window_mutex);
    hide_window();
}

void LiveView::notice_user_key_limits(int key_index, bool lower, std::string input)
{
    boost::lock_guard<boost::mutex> lock( window_mutex );
    boost::optional< boost::units::quantity<camera::intensity> > v;
    if ( input != "" )
        v = atof(input.c_str()) * camera::ad_count;
    if ( lower )
        lower_user_limit = v;
    else
        upper_user_limit = v;
}

}
}
