#include "ScaleBar.h"
#include "helpers.h"

#include "debug.h"

namespace dStorm {
namespace display {

ScaleBar::ScaleBar( wxWindow* parent, const wxSize& size )
: wxWindow( parent, wxID_ANY, wxDefaultPosition, size ),
  resolution(), zoom_factor(0)
{
}

void ScaleBar::set_pixel_size( Resolution nm_per_source_pixel ) {
    this->resolution = nm_per_source_pixel;
    wxClientDC dc(this);
    draw( dc );
}

void ScaleBar::set_zoom_factor( float zoom_factor ) {
    this->zoom_factor = zoom_factor;
    wxClientDC dc(this);
    draw( dc );
}

void ScaleBar::OnPaint( wxPaintEvent&) {
    DEBUG("Paint");
    wxPaintDC dc(this);
    draw( dc );
    DEBUG("Paint end");
}

void ScaleBar::OnResize( wxSizeEvent&) {
    DEBUG("Resize");
    wxClientDC dc(this);
    draw( dc );
    DEBUG("Resize end");
}

using namespace boost::units;
using namespace camera;

void ScaleBar::draw( wxDC &dc )
{
    dc.SetBackground( this->GetBackgroundColour() );
    dc.Clear();

    wxSize size = GetClientSize();
    dc.SetPen( *wxBLACK_PEN );
    dc.SetBrush( *wxBLACK_BRUSH );
    dc.DrawRectangle( 0, 0, size.GetWidth(), 8 );

    Resolution::PerPixel display_res = resolution.value * zoom_factor;
    if ( display_res > 0 / camera::pixel ) {
        wxChar buffer[128];
        float remainder; const wxChar *prefix;
        float value = (float(size.GetWidth()) * pixel) * display_res;
        make_SI_prefix( value, remainder, prefix );
        wxString unit(resolution.unit_symbol.c_str(), wxConvUTF8);
        wxSnprintf( buffer, 128, _T("%.3g %s%s"), remainder, prefix, unit.c_str() );
        wxString string( buffer );

        wxSize textSize = dc.GetTextExtent( string );
        dc.DrawText( string, 
            (size.GetWidth() - textSize.GetWidth()) / 2 , 10 );
    }
}

BEGIN_EVENT_TABLE(ScaleBar, wxWindow)
    EVT_PAINT(ScaleBar::OnPaint)
    EVT_SIZE(ScaleBar::OnResize)
END_EVENT_TABLE()

}
}
