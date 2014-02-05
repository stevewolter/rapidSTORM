#ifndef DSTORM_VIEWER_LIVEBACKEND_H
#define DSTORM_VIEWER_LIVEBACKEND_H

#include "viewer/ImageDiscretizer.h"
#include "viewer/Display.h"
#include "viewer/Backend.h"
#include "viewer/LiveCache.h"

#include "density_map/DensityMap.h"
#include "density_map/VirtualListener.h"
#include "display/Manager.h"
#include <boost/thread/recursive_mutex.hpp>

namespace dStorm {
namespace viewer {

class TerminalBackend;

class LiveBackend 
: public Backend,
  public dStorm::display::DataSource
{
    typedef Discretizer< LiveCache > MyDiscretizer;
    typedef density_map::DensityMap< density_map::VirtualListener<Im::Dim>, Im::Dim> Accumulator;

    Status& status;

    std::auto_ptr<dStorm::display::Change> get_changes();

    Accumulator image;
    std::auto_ptr<ColourScheme> colorizer;
    MyDiscretizer discretization;
    LiveCache cache;
    Display cia;

    friend class TerminalBackend;

    void notice_closed_data_window();
    void look_up_key_values( const PixelInfo& info,
                             std::vector<float>& targets );
    void notice_user_key_limits(int key_index, bool lower, std::string input);

  public:
    LiveBackend(std::auto_ptr< ColourScheme >, Status&);
    LiveBackend(const TerminalBackend& other, Status&);
    ~LiveBackend() ;

    output::Output& getForwardOutput() { return image; }
    void save_image(std::string filename, const Config&);

    void set_histogram_power(float power);
    void set_top_cutoff(float fraction);
    void set_job_name( const std::string& name ) 
        { cia.set_job_name( name ); }
    std::auto_ptr<Backend> change_liveness( Status& );
    Status& get_status() const { return status; }
};

}
}

#endif
