#include "ZoomSlider.h"
#include <stdexcept>
#include "debug.h"

namespace dStorm {
namespace display {

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

/** Just forwards drawn rectangle event. No action is taken. */
void ZoomSlider::drawn_rectangle( wxRect rect )
{
    if ( listener ) listener->drawn_rectangle( rect );
}
/** Just forwards mouse over pixel event. No action is taken. */
void ZoomSlider::mouse_over_pixel( wxPoint p, Color c )
{
    if ( listener ) listener->mouse_over_pixel( p, c );
}


void ZoomSlider::OnZoomChange( wxScrollEvent& event ) {
    DEBUG("OnZoomChange");
    if ( listener ) listener->zoom_changed( event.GetPosition() );
    canvas.set_zoom( event.GetPosition() );
    event.Skip();
    DEBUG("OnZoomChange end");
}

void ZoomSlider::set_zoom_listener( Canvas::Listener& listener )
{ 
    if ( this->listener != NULL ) 
      throw std::logic_error("Second listener on zoom slider unsupported.");
    this->listener = &listener; 
}

BEGIN_EVENT_TABLE(ZoomSlider, wxSlider)
    EVT_SCROLL_CHANGED( ZoomSlider::OnZoomChange ) 
    EVT_SCROLL_THUMBTRACK( ZoomSlider::OnZoomChange ) 
END_EVENT_TABLE()

}
}
