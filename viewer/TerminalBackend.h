#ifndef DSTORM_VIEWER_TERMINALBACKEND_H
#define DSTORM_VIEWER_TERMINALBACKEND_H

#include "ImageDiscretizer.h"
#include "Display.h"
#include "Backend.h"
#include "TerminalCache.h"
#include "ColourScheme.h"
#include "LiveBackend_decl.h"
#include "Status_decl.h"

#include <dStorm/outputs/BinnedLocalizations.h>
#include <dStorm/display/Manager.h>

namespace dStorm {
namespace viewer {

class TerminalBackend 
: 
  public Backend
{
  private:
    typedef ColourScheme Colorizer;
    typedef TerminalCache Cache;
    typedef Discretizer<Cache,Colorizer> MyDiscretizer;
    typedef outputs::BinnedLocalizations< density_map::VirtualListener<Im::Dim>, Im::Dim> Accumulator;

    /** Binned image with all localizations in localizationsStore.*/
    Accumulator image;
    std::auto_ptr<Colorizer> colorizer;
    /** Discretized version of \c image. */
    MyDiscretizer discretization;
    Cache cache;
    std::string window_name;
    Status& status;
    std::auto_ptr< display::Change > get_state() const;

    friend class LiveBackend<ColourScheme>;

  public:
    TerminalBackend(std::auto_ptr<Colorizer>, Status& status );
    TerminalBackend(const LiveBackend<ColourScheme>& other, Status& );
    ~TerminalBackend() ;

    output::Output& getForwardOutput() { return image; }
    void save_image(std::string filename, const Config&);

    void set_histogram_power(float power);
    void set_top_cutoff(float);
    void set_job_name( const std::string& name ) { this->window_name = name; }
    const std::string& get_job_name() const { return window_name; }

    std::auto_ptr<Backend> change_liveness( Status& );
    std::auto_ptr<dStorm::display::Change> get_result(bool with_key = true) const;
};

}
}

#endif

