#ifndef DSTORM_VIEWER_LIVEBACKEND_IMPL_H
#define DSTORM_VIEWER_LIVEBACKEND_IMPL_H

#include "debug.h"
#include "LiveBackend.h"

#include <dStorm/outputs/BinnedLocalizations.h>
#include "ColourDisplay.h"
#include "ImageDiscretizer.h"
#include "Display.h"

namespace dStorm {
namespace viewer {

template <int Hueing>
LiveBackend<Hueing>::LiveBackend(const Config& config)
: image( config.res_enh(), 1 * cs_units::camera::pixel ),
  colorizer(config),
  discretization( 4096, 
        config.histogramPower(), image(),
        colorizer),
  cache( 4096 ),
  cia( discretization, config, *this )
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
    std::string filename, bool with_key
)
{ 
    image.clean();
    cia.save_image(filename, with_key);
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
    image.set_resolution_enhancement( re ); 
}

template <int Hueing>
std::auto_ptr<dStorm::Display::Change> 
LiveBackend<Hueing>::get_changes() {
    ost::MutexLock lock( image.getMutex() );
    image.clean(); 
    return cia.get_changes();
}


}
}

#endif
