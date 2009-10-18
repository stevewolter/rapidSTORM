#include "ScaleBar.h"
#include "helpers.h"

namespace dStorm {
namespace Display {

ScaleBar::ScaleBar( wxWindow* parent, const wxSize& size )
: wxWindow( parent, wxID_ANY, wxDefaultPosition, size ),
  nm_per_source_pixel(0), zoom_factor(0)
{
}

void ScaleBar::set_pixel_size( float nm_per_source_pixel ) {
    this->nm_per_source_pixel = nm_per_source_pixel;
    wxClientDC dc(this);
    draw( dc );
}

void ScaleBar::set_zoom_factor( float zoom_factor ) {
    this->zoom_factor = zoom_factor;
    wxClientDC dc(this);
    draw( dc );
}

void ScaleBar::OnPaint( wxPaintEvent&) {
    wxPaintDC dc(this);
    draw( dc );
}

void ScaleBar::OnResize( wxSizeEvent&) {
    wxClientDC dc(this);
    draw( dc );
}

void ScaleBar::draw( wxDC &dc )
{
    dc.SetBackground( this->GetBackgroundColour() );
    dc.Clear();

    wxSize size = GetClientSize();
    dc.SetPen( *wxBLACK_PEN );
    dc.SetBrush( *wxBLACK_BRUSH );
    dc.DrawRectangle( 0, 0, size.GetWidth(), 8 );

    float displayed_pixel_size = nm_per_source_pixel * zoom_factor;
    if ( displayed_pixel_size > 1E-21 ) {
        wxChar buffer[128];
        float remainder; const wxChar *prefix;
        make_SI_prefix( displayed_pixel_size * size.GetWidth(), 
                        remainder, prefix );
        wxSnprintf( buffer, 128, _T("%.3g %sm"), remainder, prefix );
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
