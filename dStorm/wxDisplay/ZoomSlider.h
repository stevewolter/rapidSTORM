#ifndef DSTORM_DISPLAY_ZOOMSLIDER_H
#define DSTORM_DISPLAY_ZOOMSLIDER_H

#include <wx/wx.h>
#include "Canvas.h"

namespace dStorm {
namespace Display {

class ZoomSlider 
    : public wxSlider,
      public Canvas::ZoomChangeListener
{
    Canvas &canvas;
    Canvas::ZoomChangeListener* listener;
  public:
    ZoomSlider( wxWindow *parent, Canvas &canvas );
    void zoom_changed( int to );

    void OnZoomChange( wxScrollEvent& event );

    void set_zoom_listener( Canvas::ZoomChangeListener& listener )
        { this->listener = &listener; }

    DECLARE_EVENT_TABLE();
};

}
}

#endif
