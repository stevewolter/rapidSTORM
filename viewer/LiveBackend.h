#ifndef DSTORM_VIEWER_LIVEBACKEND_H
#define DSTORM_VIEWER_LIVEBACKEND_H

#include "ImageDiscretizer.h"
#include "Display.h"
#include "Backend.h"
#include "LiveCache.h"
#include "TerminalBackend_decl.h"

#include <dStorm/outputs/BinnedLocalizations.h>
#include <dStorm/helpers/DisplayManager.h>

namespace dStorm {
namespace viewer {

template <typename Hueing>
class LiveBackend 
: public Backend,
  public dStorm::Display::DataSource
{
    typedef Hueing MyColorizer;
    typedef Display< MyColorizer > MyDisplay;
    typedef LiveCache< MyDisplay > MyCache;
    typedef Discretizer< MyCache> MyDiscretizer;
    typedef outputs::BinnedLocalizations<MyDiscretizer> Accumulator;

    Config& config;
    Status& status;

    std::auto_ptr<dStorm::Display::Change> get_changes();

    Accumulator image;
    MyColorizer colorizer;
    MyDiscretizer discretization;
    MyCache cache;
    MyDisplay cia;

    friend class TerminalBackend<Hueing>;

    void notice_closed_data_window();
    void look_up_key_values( const PixelInfo& info,
                             std::vector<float>& targets );
    void notice_user_key_limits(int key_index, bool lower, std::string input);

  public:
    LiveBackend(const MyColorizer&, Config& config, Status&);
    LiveBackend(const TerminalBackend<Hueing>& other, Config& config, Status&);
    ~LiveBackend() ;

    output::Output& getForwardOutput() { return image; }
    void save_image(std::string filename, const Config&);

    void set_histogram_power(float power);
    std::auto_ptr<Backend> adapt( std::auto_ptr<Backend> self, Config&, Status& );
};

}
}

#endif
