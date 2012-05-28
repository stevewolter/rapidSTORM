#ifndef DSTORM_DISPLAY_APPWINDOW_H
#define DSTORM_DISPLAY_APPWINDOW_H

#include <wx/wx.h>
#include "dStorm/display/DataSource.h"
#include "wxManager.h"
#include "Canvas.h"
#include <boost/thread/recursive_mutex.hpp>
#include <dStorm/display/SharedDataSource.h>

namespace dStorm {
namespace display {

class ZoomSlider;
class Key;
class ScaleBar;

class Window : public wxFrame, public Canvas::Listener
{
  private:
    Canvas* canvas;
    typedef std::vector<Key*> Keys;
    Keys keys;
    ZoomSlider *zoom;
    ScaleBar *scale_bar;
    wxStaticText *position_label;

    boost::shared_ptr< SharedDataSource > data_source;

    Color background;

    bool close_on_completion, notify_for_zoom, has_3d;

    DECLARE_EVENT_TABLE();

    void OnLowerLimitChange(wxCommandEvent&);
    void OnUpperLimitChange(wxCommandEvent&);
    void UserClosedWindow(wxCloseEvent&);

    template <typename Drawer>
    void draw_image_window( const Change& changes );
    void commit_changes(const Change& changes);

    void drawn_rectangle( wxRect rect );
    void zoom_changed( int to );
    void mouse_over_pixel( wxPoint, Color );

  public:
    Window( const Manager::WindowProperties& props,
            boost::shared_ptr< SharedDataSource > );
    ~Window(); 

    void update_image();
    boost::shared_ptr<const Change> detach_from_source();
    void notice_that_source_has_disappeared();

    std::auto_ptr<Change> getState();
};

}
}

#endif
