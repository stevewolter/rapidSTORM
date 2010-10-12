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

namespace AndorCamera {

class LiveView :
    boost::noncopyable, public simparm::Object,
    public dStorm::Display::DataSource
{
    typedef boost::units::quantity
            <cs_units::camera::resolution, float> 
        Resolution;

    boost::units::quantity<cs_units::camera::frame_rate> cycle_time;
    Resolution resolution;
    simparm::BoolEntry show_live;

    ost::Mutex window_mutex, change_mutex;
    CamImage current_image_content;

    std::auto_ptr<dStorm::Display::Change> change;
    std::auto_ptr<dStorm::Display::Manager::WindowHandle> window;

    void registerNamedEntries();

    void show_window(CamImage::Size size);
    void hide_window();

    void compute_image_change( const CamImage& image );
    void compute_key_change( CameraPixel darkest,
                             CameraPixel brightest );

    std::auto_ptr<dStorm::Display::Change> get_changes();
    void notice_closed_data_window();

  public:
    LiveView(
        bool on_by_default,
        Resolution resolution,
        boost::units::quantity<cs_units::camera::frame_rate> cycle_time );
    ~LiveView();
    void show( const CamImage& image, int num );
};

}

#endif
