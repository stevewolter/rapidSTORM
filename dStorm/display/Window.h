#ifndef DSTORM_DISPLAY_APPWINDOW_H
#define DSTORM_DISPLAY_APPWINDOW_H

#include <wx/wx.h>
#include "DataSource.h"
#include "wxManager.h"
#include "Canvas.h"

namespace dStorm {
namespace Display {

class ZoomSlider;
class Key;
class ScaleBar;

class Window : public wxFrame, public Canvas::ZoomChangeListener
{
  private:
    Canvas* canvas;
    Key* key;
    ZoomSlider *zoom;
    ScaleBar *scale_bar;
    wxTimer timer;

    DataSource* source;
    wxManager::WindowHandle *handle;

    DECLARE_EVENT_TABLE();

    void OnTimer(wxTimerEvent& event);

    template <typename Drawer>
    void draw_image_window( const Change& changes );
    void commit_changes(const Change& changes);

    void zoom_changed( int to );

  public:
    Window( const Manager::WindowProperties& props,
            DataSource* data_source,
            wxManager::WindowHandle *my_handle );
    ~Window(); 

    void remove_data_source();
};

}
}

#endif
