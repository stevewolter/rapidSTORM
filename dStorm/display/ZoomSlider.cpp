#include "ZoomSlider.h"

namespace dStorm {
namespace Display {

ZoomSlider::ZoomSlider( wxWindow *parent, Canvas &canvas )
: wxSlider( parent, wxID_ANY, 0, -16, 16, 
            wxDefaultPosition, wxDefaultSize,
            wxSL_AUTOTICKS | wxSL_LABELS ),
  canvas(canvas),
  listener(NULL)
{
    SetLineSize( 5 );
    SetTickFreq( 1, 1 );
    canvas.set_listener( this );
}

void ZoomSlider::zoom_changed( int to ) {
    if ( GetValue() != to ) {
        SetValue( to );
        if ( listener ) listener->zoom_changed( to );
    }
}

void ZoomSlider::OnZoomChange( wxScrollEvent& event ) {
    if ( listener ) listener->zoom_changed( event.GetPosition() );
    canvas.set_zoom( event.GetPosition() );
    event.Skip();
}

BEGIN_EVENT_TABLE(ZoomSlider, wxSlider)
    EVT_SCROLL_CHANGED( ZoomSlider::OnZoomChange ) 
    EVT_SCROLL_THUMBTRACK( ZoomSlider::OnZoomChange ) 
END_EVENT_TABLE()

}
}
