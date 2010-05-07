#ifndef ANDORCAMERA_LIVEVIEW_H
#define ANDORCAMERA_LIVEVIEW_H

#include "AndorDirect_decl.h"
#include <boost/utility.hpp>
#include <boost/units/quantity.hpp>
#include <simparm/Object.hh>
#include <dStorm/helpers/DisplayManager.h>
#include <cs_units/camera/resolution.hpp>
#include <cs_units/camera/frame_rate.hpp>
#include <dStorm/helpers/thread.h>
#include <simparm/NumericEntry.hh>

namespace dStorm {
namespace AndorDirect {

class LiveView :
    boost::noncopyable, public simparm::Object,
    public Display::DataSource
{
    typedef boost::units::quantity
            <cs_units::camera::resolution, float> 
        Resolution;

    boost::units::quantity<cs_units::camera::frame_rate> cycle_time;
    Resolution resolution;
    simparm::BoolEntry show_live;
    simparm::DoubleEntry live_show_frequency;

    ost::Mutex window_mutex, change_mutex;

    std::auto_ptr<Display::Change> change;
    std::auto_ptr<Display::Manager::WindowHandle> window;

    void registerNamedEntries();

    void show_window(int width, int height);
    void hide_window();

    void compute_image_change( const CamImage& image );
    void compute_key_change( CameraPixel darkest,
                             CameraPixel brightest );

    std::auto_ptr<Display::Change> get_changes();
    void notice_closed_data_window();

  public:
    LiveView(
        const Config& config,
        boost::units::quantity<cs_units::camera::frame_rate> cycle_time );
    void show( const CamImage& image, int num );
};

}
}

#endif
