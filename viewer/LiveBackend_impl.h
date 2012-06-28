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
  image( s.config.binned_dimensions.make(), s.config.interpolator.make(), s.config.crop_border() ),
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
    DEBUG("Constructed " << this);
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
    discretization.setHistogramPower( power ); 
}

template <typename Hueing>
void LiveBackend<Hueing>::set_top_cutoff(float cutoff) {
    discretization.set_top_cutoff(cutoff);
}

template <typename Hueing>
std::auto_ptr<dStorm::display::Change> 
LiveBackend<Hueing>::get_changes() {
    boost::lock_guard<boost::mutex> lock( status.mutex );
    image.clean(); 
    return cia.get_changes();
}

template <typename Hueing>
void LiveBackend<Hueing>::notice_closed_data_window() {
    DEBUG( "Noticing closed data window in " << this );
    status.config.showOutput = false;
    DEBUG( "Noticed closed data window in " << this );
}

template <typename Hueing>
void LiveBackend<Hueing>::look_up_key_values(
    const PixelInfo& info,
    std::vector<float>& targets )
{
    DEBUG("Looking up key values in " << this);
    boost::lock_guard<boost::mutex> lock( status.mutex );
    if ( ! targets.empty() ) {
        assert( image().is_valid() );
        const dStorm::Image<float,Im::Dim>& im = image();
        if ( im.contains( info ) ) {
            targets[0] = im(info);
        }
    }
}

template <typename Hueing>
void LiveBackend<Hueing>::notice_user_key_limits(int key_index, bool lower, std::string input)
{
    if ( key_index == 0 )
        throw std::runtime_error("Intensity key cannot be limited");
    else {
        boost::lock_guard<boost::mutex> lock( status.mutex );
        colorizer.notice_user_key_limits(key_index, lower, input);
    }
}

}
}

#endif
