#ifndef DSTORM_VIEWER_LIVEBACKEND_H
#define DSTORM_VIEWER_LIVEBACKEND_H

#include "ColourDisplay.h"
#include "ImageDiscretizer.h"
#include "Display.h"
#include "Backend.h"
#include "LiveCache.h"

#include <dStorm/outputs/BinnedLocalizations.h>
#include <dStorm/helpers/DisplayManager.h>

namespace dStorm {
namespace viewer {

template <int Hueing>
class LiveBackend 
: public Backend,
  public dStorm::Display::DataSource
{
    typedef HueingColorizer<Hueing> MyColorizer;
    typedef Display< MyColorizer > MyDisplay;
    typedef LiveCache< MyDisplay > MyCache;
    typedef Discretizer< MyCache> MyDiscretizer;
    typedef outputs::BinnedLocalizations<MyDiscretizer> Accumulator;

    std::auto_ptr<dStorm::Display::Change> get_changes();

    Accumulator image;
    MyColorizer colorizer;
    MyDiscretizer discretization;
    MyCache cache;
    MyDisplay cia;

  public:
    LiveBackend(const Config& config);
    ~LiveBackend() ;

    output::Output& getForwardOutput() { return image; }
    void save_image(std::string filename, bool with_key);

    void set_histogram_power(float power);
    void set_resolution_enhancement(float re);
};

std::auto_ptr<Backend>
select_live_backend( const Config& config );

}
}

#endif
