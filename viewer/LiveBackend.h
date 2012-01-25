#ifndef DSTORM_VIEWER_LIVEBACKEND_H
#define DSTORM_VIEWER_LIVEBACKEND_H

#include "ImageDiscretizer.h"
#include "Display.h"
#include "Backend.h"
#include "LiveCache.h"
#include "TerminalBackend_decl.h"

#include <dStorm/outputs/BinnedLocalizations.h>
#include <dStorm/display/Manager.h>
#include <boost/thread/recursive_mutex.hpp>

namespace dStorm {
namespace viewer {

template <typename Hueing>
class LiveBackend 
: public Backend,
  public dStorm::display::DataSource
{
    typedef Hueing MyColorizer;
    typedef Display< MyColorizer > MyDisplay;
    typedef LiveCache< MyDisplay > MyCache;
    typedef Discretizer< MyCache, MyColorizer > MyDiscretizer;
    typedef outputs::BinnedLocalizations<MyDiscretizer, Im::Dim> Accumulator;

    Status& status;

    boost::recursive_mutex* mutex;
    std::auto_ptr<dStorm::display::Change> get_changes();

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
    LiveBackend(const MyColorizer&, Status&);
    LiveBackend(const TerminalBackend<Hueing>& other, Status&);
    ~LiveBackend() ;

    output::Output& getForwardOutput() { return image; }
    void save_image(std::string filename, const Config&);

    void set_histogram_power(float power);
    void set_output_mutex( boost::recursive_mutex* mutex ) 
        { this->mutex = mutex; cia.show_window(); }
    void set_job_name( const std::string& name ) 
        { cia.set_job_name( name ); }
    std::auto_ptr<Backend> change_liveness( Status& );
    Status& get_status() const { return status; }
};

}
}

#endif
