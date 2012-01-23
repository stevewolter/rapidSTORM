#ifndef DSTORM_VIEWER_TERMINALBACKEND_H
#define DSTORM_VIEWER_TERMINALBACKEND_H

#include "ImageDiscretizer.h"
#include "Display.h"
#include "Backend.h"
#include "TerminalCache.h"
#include "LiveBackend_decl.h"
#include "Status_decl.h"

#include <dStorm/outputs/BinnedLocalizations.h>
#include <dStorm/display/Manager.h>

namespace dStorm {
namespace viewer {

template <typename Hueing>
class TerminalBackend 
: 
  public Backend
{
  private:
    typedef Hueing Colorizer;
    typedef TerminalCache<Colorizer> Cache;
    typedef Discretizer<Cache> MyDiscretizer;
    typedef outputs::BinnedLocalizations<MyDiscretizer, Im::Dim> Accumulator;

    /** Binned image with all localizations in localizationsStore.*/
    Accumulator image;
    Colorizer colorizer;
    /** Discretized version of \c image. */
    MyDiscretizer discretization;
    Cache cache;
    std::string window_name;
    Status& status;

    friend class LiveBackend<Hueing>;

  public:
    TerminalBackend(const Colorizer&, Status& status );
    TerminalBackend(const LiveBackend<Hueing>& other, Status& );
    ~TerminalBackend() ;

    output::Output& getForwardOutput() { return image; }
    void save_image(std::string filename, const Config&);

    void set_histogram_power(float power);
    void set_job_name( const std::string& name ) { this->window_name = name; }
    const std::string& get_job_name() const { return window_name; }

    std::auto_ptr<Backend> adapt( std::auto_ptr<Backend> self, Status& );
    std::auto_ptr<dStorm::display::Change> get_result(bool with_key = true) const;
};

}
}

#endif

