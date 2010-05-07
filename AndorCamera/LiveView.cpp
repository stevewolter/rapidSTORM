#include "LiveView.h"
#include <boost/thread/locks.hpp>
#include "AndorDirect.h"
#include <CImg.h>

typedef ost::MutexLock guard;

using namespace boost::units;
using namespace cs_units::camera;

namespace dStorm {
namespace AndorDirect {

LiveView::LiveView( 
    const Config& config,
    boost::units::quantity<cs_units::camera::frame_rate> cycle_time
    )
: Object("LiveView", "Live view options"),
  cycle_time( cycle_time ),
  resolution( 1E9 * cs_units::camera::pixels_per_meter
               / config.resolution_element() ),
  show_live("ShowLive", "Show camera image", 
            config.show_live_by_default()),
  live_show_frequency( config.live_show_frequency ),
  change( new Display::Change() )
{
    registerNamedEntries();
}

void LiveView::registerNamedEntries() {
    push_back( show_live );
    push_back( live_show_frequency );
}

void LiveView::show_window(int width, int height) {
    if ( window.get() == NULL ) {
        Display::Manager::WindowProperties props;
        props.name = "Live camera view";
        props.flags.close_window_on_unregister();
        props.initial_size.width = width;
        props.initial_size.height = height;
        props.initial_size.key_size = 256;
        props.initial_size.pixel_size = 
            (1.0 * cs_units::camera::pixels_per_meter / resolution);

        window = Display::Manager::getSingleton()
            .register_data_source( props, *this );
    }
}

void LiveView::hide_window() {
    if ( window.get() != NULL ) {
        show_live = false;
        window.reset( NULL );
    }
}

std::auto_ptr<Display::Change> LiveView::get_changes()
{
    std::auto_ptr<Display::Change> fresh( new Display::Change() );
    ost::MutexLock lock(change_mutex);
    std::swap( fresh, change );
    return fresh;
}

void LiveView::compute_image_change
    ( const CamImage& image )
{
    CameraPixel minPix, maxPix;
    minPix = image.minmax(maxPix);

    cimg_library::CImg<uint8_t> normalized( 
        image.get_normalize(0, 255) );

    guard lock( change_mutex );
    change->do_change_image = true;
    change->image_change.pixels.resize(normalized.size());
    cimg_foroff( normalized, off )
        change->image_change.pixels[off] = normalized[off];
    compute_key_change( minPix, maxPix );
}

/** Assumes that change_mutex is already acquired. */
void LiveView::compute_key_change(
    CameraPixel darkest, CameraPixel brightest )
{
    Display::KeyChange *v = change->change_key.allocate( 256 );
    for (int i = 0; i <= 255; i++) {
        v[i].index = i;
        v[i].color = i;
        v[i].value = darkest + i * ((brightest - darkest) / 255.0);
    }
    change->change_key.commit(256);
}

void LiveView::show( const CamImage& image, int number) {
    guard lock(window_mutex);
    if ( show_live() ) {
        if ( window.get() == NULL ) 
            show_window(image.width, image.height);

        quantity<si::time>
            image_time = frame / cycle_time,
            show_time = live_show_frequency() * si::seconds;

        int show_cycle = std::max(1, int(round( show_time / image_time)));
        if ( number % show_cycle == 0 )
            compute_image_change( image );
    } else {
        if ( window.get() != NULL ) hide_window();
    }
}

void LiveView::notice_closed_data_window() {
    guard lock(window_mutex);
    hide_window();
}

}
}
