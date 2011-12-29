#ifndef DSTORM_DISPLAYHANDLER_SCALEBAR_H
#define DSTORM_DISPLAYHANDLER_SCALEBAR_H

#include <wx/wx.h>
#include <boost/units/quantity.hpp>
#include <dStorm/display/DataSource.h>

namespace dStorm {
namespace display {

class ScaleBar
: public wxWindow
{
    DECLARE_EVENT_TABLE();

    typedef ResizeChange::Resolution Resolution;
    Resolution resolution;
    float zoom_factor;

    void draw( wxDC& dc );

  public:
    ScaleBar( wxWindow* parent, const wxSize& size );

    void set_pixel_size( Resolution nm_per_source_pixel );
    Resolution get_pixel_size() 
        { return resolution; }
    void set_zoom_factor( float zoom_factor );

    void OnPaint(wxPaintEvent&);
    void OnResize(wxSizeEvent&);
};

}
}

#endif
