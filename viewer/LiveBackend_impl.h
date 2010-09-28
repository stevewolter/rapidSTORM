#ifndef DSTORM_VIEWER_LIVEBACKEND_IMPL_H
#define DSTORM_VIEWER_LIVEBACKEND_IMPL_H

#include "debug.h"
#include "LiveBackend.h"

#include <dStorm/outputs/BinnedLocalizations.h>
#include "ColourDisplay.h"
#include "ImageDiscretizer.h"
#include "Display.h"
#include "Status.h"

namespace dStorm {
namespace viewer {

template <int Hueing>
LiveBackend<Hueing>::LiveBackend(Config& config, Status& s)
: config(config), status(s), 
  image( config.res_enh(), config.border() ),
  colorizer(config),
  discretization( 4096, 
        config.histogramPower(), image(),
        colorizer),
  cache( 4096 ),
  cia( discretization, config, *this, colorizer )
{
    image.setListener(&discretization);
    discretization.setListener(&cache);
    cache.setListener(&cia);
}

template <int Hueing>
LiveBackend<Hueing>::~LiveBackend() {
    DEBUG("Destructing viewer implementation");
}

template <int Hueing>
void LiveBackend<Hueing>::save_image(
    std::string filename, const Config& config
)
{ 
    image.clean();
    cia.save_image(filename, config);
}

template <int Hueing>
void LiveBackend<Hueing>::set_histogram_power(float power) {
    /* The \c image member is not involved here, so we have to lock
        * it ourselves. */
    ost::MutexLock lock( image.getMutex() );
    discretization.setHistogramPower( power ); 
}

template <int Hueing>
void LiveBackend<Hueing>::set_resolution_enhancement(float re)  { 
    ost::MutexLock lock( image.getMutex() );
    image.set_resolution_enhancement( re ); 
}

template <int Hueing>
std::auto_ptr<dStorm::Display::Change> 
LiveBackend<Hueing>::get_changes() {
    image.clean(); 
    ost::MutexLock lock( image.getMutex() );
    return cia.get_changes();
}

template <int Hueing>
void LiveBackend<Hueing>::notice_closed_data_window() {
    DEBUG("Noticing closed data window");
    config.showOutput = false;
    status.adapt_to_changed_config();
    DEBUG("Noticed closed data window");
}

}
}

#endif
