#ifndef DSTORM_VIEWER_LIVEBACKEND_IMPL_H
#define DSTORM_VIEWER_LIVEBACKEND_IMPL_H

#include "debug.h"
#include "LiveBackend.h"

#include <dStorm/outputs/BinnedLocalizations.h>
#include "ImageDiscretizer.h"
#include "Display.h"
#include "Status.h"
#include "Config.h"

namespace dStorm {
namespace viewer {

template <typename Hueing>
LiveBackend<Hueing>::LiveBackend(const MyColorizer& col, Status& s)
: status(s), 
  mutex( NULL ),
  image( s.config.binned_dimensions.make(), s.config.crop_border() ),
  colorizer(col),
  discretization( 4096, 
        s.config.histogramPower(), image(),
        colorizer),
  cache( 4096 ),
  cia( discretization, s, *this, colorizer )
{
    image.setListener(&discretization);
    discretization.setListener(&cache);
    cache.setListener(&cia);
}

template <typename Hueing>
LiveBackend<Hueing>::~LiveBackend() {
    DEBUG("Destructing viewer implementation " << this);
}

template <typename Hueing>
void LiveBackend<Hueing>::save_image(
    std::string filename, const Config& config
)
{ 
    image.clean();
    cia.save_image(filename, config);
}

template <typename Hueing>
void LiveBackend<Hueing>::set_histogram_power(float power) {
    /* The \c image member is not involved here, so we have to lock
        * it ourselves. */
    discretization.setHistogramPower( power ); 
}

template <typename Hueing>
std::auto_ptr<dStorm::display::Change> 
LiveBackend<Hueing>::get_changes() {
    boost::lock_guard<boost::recursive_mutex> lock( *mutex );
    image.clean(); 
    return cia.get_changes();
}

template <typename Hueing>
void LiveBackend<Hueing>::notice_closed_data_window() {
    DEBUG( "Noticing closed data window in " << this );
    boost::lock_guard<boost::recursive_mutex> lock( *mutex );
    status.config.showOutput = false;
    DEBUG( "Noticed closed data window in " << this );
}

template <typename Hueing>
void LiveBackend<Hueing>::look_up_key_values(
    const PixelInfo& info,
    std::vector<float>& targets )
{
    boost::lock_guard<boost::recursive_mutex> lock( *mutex );
    if ( ! targets.empty() ) {
        const dStorm::Image<float,Im::Dim>& im = image();
        if ( im.width_in_pixels() > info.x() && im.height_in_pixels() > info.y() ) {
            targets[0] = im(info.head<Im::Dim>());
        }
    }
}

template <typename Hueing>
void LiveBackend<Hueing>::notice_user_key_limits(int key_index, bool lower, std::string input)
{
    if ( key_index == 0 )
        throw std::runtime_error("Intensity key cannot be limited");
    else {
        boost::lock_guard<boost::recursive_mutex> lock( *mutex );
        colorizer.notice_user_key_limits(key_index, lower, input);
    }
}

}
}

#endif
