#ifndef DSTORM_DISPLAYHANDLER_SCALEBAR_H
#define DSTORM_DISPLAYHANDLER_SCALEBAR_H

#include <wx/wx.h>

namespace dStorm {
namespace Display {

class ScaleBar
: public wxWindow
{
    DECLARE_EVENT_TABLE();

    float nm_per_source_pixel;
    float zoom_factor;

    void draw( wxDC& dc );

  public:
    ScaleBar( wxWindow* parent, const wxSize& size );

    void set_pixel_size( float nm_per_source_pixel );
    void set_zoom_factor( float zoom_factor );

    void OnPaint(wxPaintEvent&);
    void OnResize(wxSizeEvent&);
};

}
}

#endif
