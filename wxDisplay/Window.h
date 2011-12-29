#ifndef DSTORM_DISPLAY_APPWINDOW_H
#define DSTORM_DISPLAY_APPWINDOW_H

#include <wx/wx.h>
#include "dStorm/display/DataSource.h"
#include "wxManager.h"
#include "Canvas.h"

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

    DataSource* source;
    wxManager::WindowHandle *handle;

    Color background;

    bool close_on_completion, notify_for_zoom;

    DECLARE_EVENT_TABLE();

    void OnLowerLimitChange(wxCommandEvent&);
    void OnUpperLimitChange(wxCommandEvent&);

    template <typename Drawer>
    void draw_image_window( const Change& changes );
    void commit_changes(const Change& changes);

    void drawn_rectangle( wxRect rect );
    void zoom_changed( int to );
    void mouse_over_pixel( wxPoint, Color );

  public:
    Window( const Manager::WindowProperties& props,
            DataSource* data_source,
            wxManager::WindowHandle *my_handle );
    ~Window(); 

    void update_image();
    void remove_data_source();

    std::auto_ptr<Change> getState();
};

}
}

#endif
