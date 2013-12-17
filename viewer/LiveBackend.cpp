#include "LiveBackend.h"

#include <boost/thread/lock_guard.hpp>

#include "ImageDiscretizer.h"
#include "ImageDiscretizer_converter.h"
#include "Display.h"
#include "Status.h"
#include "Config.h"
#include "ColourScheme.h"
#include "TerminalBackend.h"
#include "density_map/Coordinates.h"

namespace dStorm {
namespace viewer {

LiveBackend::LiveBackend(std::auto_ptr< ColourScheme > col, Status& s)
: status(s), 
  image( NULL, s.config.binned_dimensions.make(), s.config.interpolator.make(), s.config.crop_border() ),
  colorizer(col),
  discretization( 4096, 
        s.config.histogramPower(), image(),
        *colorizer),
  cache( 4096 ),
  cia( discretization, s, *this, *colorizer )
{
    image.set_listener(&discretization);
    discretization.setListener(&cache);
    cache.setListener(&cia);
}

LiveBackend::LiveBackend(const TerminalBackend& other, Status& s)
: status(s), 
  image( other.image ),
  colorizer( other.colorizer->clone() ),
  discretization( other.discretization,
                  image(), *colorizer ),
  cache( 4096, other.cache.getSize().size ),
  cia( discretization, s, *this, *colorizer, other.get_result() )
{
    cia.set_job_name( other.get_job_name() );
    image.set_listener(&discretization);
    discretization.setListener(&cache);
    cache.setListener(&cia);
    cia.show_window();
}

LiveBackend::~LiveBackend() {
}

void LiveBackend::save_image(
    std::string filename, const Config& config
)
{ 
    image.clean();
    cia.save_image(filename, config);
}

void LiveBackend::set_histogram_power(float power) {
    discretization.setHistogramPower( power ); 
}

void LiveBackend::set_top_cutoff(float cutoff) {
    discretization.set_top_cutoff(cutoff);
}

std::auto_ptr<dStorm::display::Change> 
LiveBackend::get_changes() {
    boost::lock_guard<boost::mutex> lock( status.mutex );
    image.clean(); 
    return cia.get_changes();
}

void LiveBackend::notice_closed_data_window() {
    status.config.showOutput = false;
}

void LiveBackend::look_up_key_values(
    const PixelInfo& info,
    std::vector<float>& targets )
{
    boost::lock_guard<boost::mutex> lock( status.mutex );
    if ( ! targets.empty() ) {
        assert( image().is_valid() );
        const dStorm::Image<float,Im::Dim>& im = image();
        if ( im.contains( info ) ) {
            targets[0] = im(info);
        }
    }
}

void LiveBackend::notice_user_key_limits(int key_index, bool lower, std::string input)
{
    if ( key_index == 0 )
        throw std::runtime_error("Intensity key cannot be limited");
    else {
        boost::lock_guard<boost::mutex> lock( status.mutex );
        colorizer->notice_user_key_limits(key_index, lower, input);
    }
}

std::auto_ptr<Backend>
LiveBackend::change_liveness( Status& s ) {
    if ( ! s.config.showOutput() ) {
        return std::auto_ptr<Backend>( new TerminalBackend(*this, s) );
    } else {
        return std::auto_ptr<Backend>();
    }
}

}
}
