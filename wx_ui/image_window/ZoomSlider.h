#ifndef DSTORM_DISPLAY_ZOOMSLIDER_H
#define DSTORM_DISPLAY_ZOOMSLIDER_H

#include <wx/wx.h>
#include "Canvas.h"

namespace simparm {
namespace wx_ui {
namespace image_window {

class ZoomSlider 
    : public wxSlider,
      public Canvas::Listener
{
    Canvas &canvas;
    Canvas::Listener* listener;
  public:
    ZoomSlider( wxWindow *parent, Canvas &canvas );
    void zoom_changed( int to );
    void drawn_rectangle( wxRect rect );
    void mouse_over_pixel( wxPoint, Color );

    void OnZoomChange( wxScrollEvent& event );

    void set_zoom_listener( Canvas::Listener& listener );

    DECLARE_EVENT_TABLE();
};

}
}
}

#endif
