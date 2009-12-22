#include "Canvas.h"
#include <stdexcept>
#include <wx/dcbuffer.h>

namespace dStorm {
namespace Display {

wxColor makeColor( const dStorm::Display::Color& c ) {
    return wxColor( c.r, c.g, c.b );
}

IMPLEMENT_DYNAMIC_CLASS(Canvas, wxScrolledWindow);

BEGIN_EVENT_TABLE(Canvas, wxScrolledWindow)
    EVT_PAINT(Canvas::OnPaint)
    EVT_LEFT_DOWN(Canvas::OnMouseDown)
    EVT_LEFT_UP(Canvas::OnMouseUp)
    EVT_MOTION(Canvas::OnMouseMotion)
END_EVENT_TABLE()

Canvas::Canvas( 
    wxWindow *parent, wxWindowID id,
    const wxSize& init_size )
: wxScrolledWindow(parent, id, wxDefaultPosition, wxDefaultSize),
  zcl(NULL), 
  contents( 
    new wxImage(init_size.GetWidth(), init_size.GetHeight(), false) ), 
  zoom_in_level( 1 ), zoom_out_level( 1 ),
  mouse_state( Moving )
{
    SetScrollRate( 10, 10 );
    wxScrolledWindow::SetVirtualSize( init_size );
    SetBackgroundColour( *wxLIGHT_GREY );
}

void Canvas::set_listener(Listener* listener)
{
    if ( zcl != NULL )
      throw std::logic_error("Second listener on zoom slider unsupported.");
    zcl = listener;
}

int Canvas::transform( int coords, int mult,
    int div, Bound bound )
{
    int rv = coords * mult / div;
    if ( mult > 1 && bound == Upper )
        rv += mult - 1;
    return rv;
}

int Canvas::transform( bool mode, Bound bound,
                        int coord )
{
    return transform( coord, 
        ( mode ) ? zoom_out_level : zoom_in_level,
        ( mode ) ? zoom_in_level : zoom_out_level,
        bound );
}

const wxRect Canvas::transformed_coords(
    const wxRect& conversion, bool to_image_coords )
{
    int left = transform( to_image_coords, Lower, conversion.x );
    int top = transform( to_image_coords, Lower, conversion.y );
    int right = transform( to_image_coords, Upper, conversion.GetRight() ),
        bottom = transform( to_image_coords, Upper, conversion.GetBottom() );
    return wxRect( left, top, right - left + 1, bottom - top + 1 );
}

const wxPoint Canvas::image_coords( const wxPoint& p ) 
{
    return wxPoint( transform( true, Lower, p.x ),
                    transform( true, Lower, p.y ) );
}

const wxRect Canvas::image_coords( const wxRect& canvas_coords ) 
{
    return transformed_coords( canvas_coords, true );
}

const wxPoint Canvas::canvas_coords( const wxPoint& p ) 
{
    return wxPoint( transform( false, Lower, p.x ),
                    transform( false, Lower, p.y ) );
}

const wxRect Canvas::canvas_coords( const wxRect& image_coords ) 
{
    return transformed_coords( image_coords, false );
}


wxRect Canvas::get_visible_region() {
    int xl, yl, xs, ys, xr, yr;
    GetScrollPixelsPerUnit( &xr, &yr );
    GetClientSize( &xs, &ys );
    GetViewStart( &xl, &yl );

    wxSize image_size =
        wxSize( contents->GetWidth() , contents->GetHeight() ) 
            * zoom_in_level / zoom_out_level;

    xs = std::min( xs, image_size.GetWidth() - xl * xr );
    ys = std::min( ys, image_size.GetWidth() - yl * yr );
    return wxRect( xl * xr, yl * yr, xs, ys );
}

wxRect Canvas::get_rect_by_corners
    ( wxPoint a, wxPoint b ) 
{
    /* Bring to unscrolled coordinates */
    CalcUnscrolledPosition( a.x, a.y, &a.x, &a.y );
    CalcUnscrolledPosition( b.x, b.y, &b.x, &b.y );
    /* Find upper left and lower right corners. */
    if ( a.x > b.x )
        std::swap( a.x, b.x );
    if ( a.y > b.y )
        std::swap( a.y, b.y );
    return wxRect( a, b );
}

void Canvas::draw_rectangle(wxRect rect, wxDC *dc) {
    dc->SetPen( *wxMEDIUM_GREY_PEN );
    dc->SetBrush( *wxTRANSPARENT_BRUSH );
    dc->DrawRectangle( rect );
}

void Canvas::overdraw_rectangle(wxRect rect, wxDC *dc) 
{
    wxRect edges[4];
    wxSize hor(rect.GetWidth(), 1), ver(1, rect.GetHeight());
    edges[0] = wxRect( rect.GetTopLeft(), hor );
    edges[1] = wxRect( rect.GetTopLeft(), ver );
    edges[2] = wxRect( rect.GetTopRight(), ver );
    edges[3] = wxRect( rect.GetBottomLeft(), hor );

    for (int i = 0; i < 4; i++) {
        dc->DrawBitmap( zoomed_bitmap_for_canvas_region( edges[i] ),
                    edges[i].GetTopLeft() );
    }
}

void Canvas::draw_zoom_rectangle( wxDC *dc )
{
    wxRect drawn_rect = get_rect_by_corners( drag_start, drag_end );
    wxRect max_rect( 
        wxSize( contents->GetWidth(), contents->GetHeight() )
            * zoom_in_level / zoom_out_level );
    if ( drawn_rect.Intersects( max_rect ) )
        draw_rectangle( drawn_rect.Intersect( max_rect ), dc );
}

void Canvas::zoom_to( wxRect rect )
{
    wxSize window_size = wxWindow::GetClientSize();
    float ratio =
        std::max( float(rect.GetWidth()) / window_size.GetWidth(),
                    float(rect.GetHeight()) / window_size.GetHeight() );

    int zoom;
    if ( ratio < 1 ) {
        /* Zoom in. */
        zoom = round( 1 / ratio ) - 1;
    } else {
        /* Zoom out. */
        zoom = - round( ratio );
    }
    zoom = std::max( -16, std::min( zoom, 16 ) );

    wxPoint sumPoint = (rect.GetTopLeft() + rect.GetBottomRight());
    wxPoint centerPoint( sumPoint.x/2, sumPoint.y/2);
    set_zoom( zoom, centerPoint );
}
void Canvas::OnPaint( wxPaintEvent & ) {
    wxPaintDC dc(this);
    if ( contents->GetWidth() <= 0 ) return;
    wxScrolledWindow::PrepareDC(dc);

    wxRegion updateRegions = GetUpdateRegion();
    for ( wxRegionIterator region( updateRegions ); region; ++region ) {
        wxRect rect = region.GetRect();
        CalcUnscrolledPosition( rect.x, rect.y, &rect.x, &rect.y );

        /* Find the actual image part of the update region. */
        wxRect image_area = wxRect( wxSize(contents->GetWidth(),
                                           contents->GetHeight()) );
        rect.Intersect( canvas_coords( image_area ) );
        dc.DrawBitmap( zoomed_bitmap_for_canvas_region( rect ),
                       rect.GetTopLeft() );
    }
}

wxBitmap Canvas::zoomed_bitmap_for_canvas_region
    ( const wxRect& unclipped_rect )
{
    wxRect image_area = wxRect( wxSize(contents->GetWidth(),
                                       contents->GetHeight()) );
    wxRect rect = unclipped_rect.Intersect( canvas_coords( image_area ) );

    wxRect subimage = image_coords( rect ), subcanvas = rect;
    wxRect smoothing_region_in_image = subimage,
            smoothing_region_in_canvas;

    smoothing_region_in_image.Inflate( Overlap * zoom_out_level )
        .Intersect( image_area );

    smoothing_region_in_canvas = 
        canvas_coords( smoothing_region_in_image );

    wxImage toScale = contents->GetSubImage( smoothing_region_in_image );
    toScale.Rescale( smoothing_region_in_canvas.GetWidth(), 
                        smoothing_region_in_canvas.GetHeight() );
    wxRect update_region_in_enlarged( rect );
    update_region_in_enlarged.Offset( 
        - smoothing_region_in_canvas.GetTopLeft() );
    wxImage toDraw = toScale.GetSubImage( update_region_in_enlarged );
    
    return wxBitmap( toDraw );
}

void Canvas::resize( const wxSize& new_size ) {
    contents.reset( NULL );
    contents.reset( new wxImage(
        new_size.GetWidth(), new_size.GetHeight() ) );
    wxScrolledWindow::SetVirtualSize( canvas_coords( new_size ).GetSize() );

    zoom_to( canvas_coords( new_size ) );
}

void Canvas::DirectDrawer::draw( int x, int y, const Color& co ) {
    BufferedDrawer::draw( x, y, co );

    wxRect visible_region = c.get_visible_region();
    wxRect image_region( x, y, 1, 1 );
    wxRect canvas_region = c.canvas_coords( image_region );
    canvas_region.Inflate( Overlap );
    if ( canvas_region.Intersects( visible_region ) ) {
        wxBitmap bitmap = c.zoomed_bitmap_for_canvas_region( canvas_region );
        dc.DrawBitmap( bitmap, canvas_region.GetTopLeft() );
    }
}

void Canvas::BufferedDrawer::clear( const Color &color ) {
    unsigned char *ptr = c.contents->GetData();
    int size = 3 * c.contents->GetWidth() * c.contents->GetHeight();
    for (int i = 0; i < size; i += 3) {
        ptr[i] = color.r;
        ptr[i+1] = color.g;
        ptr[i+2] = color.b;
    }
}

void Canvas::DirectDrawer::clear( const Color &color ) {
    wxPen pen( makeColor(color) );
    wxBrush brush( makeColor(color) );
    dc.SetPen( pen );
    dc.SetBrush( brush );
    dc.DrawRectangle( c.canvas_coords(
        wxRect(0, 0, c.contents->GetWidth(), c.contents->GetHeight())) );

    BufferedDrawer::clear( color );
}

void Canvas::BufferedDrawer::finish() {
    wxClientDC dc(&c);
    c.DoPrepareDC( dc );

    wxRect canvas_region = c.get_visible_region();
    wxBitmap bitmap = c.zoomed_bitmap_for_canvas_region( canvas_region );
    dc.DrawBitmap( bitmap, canvas_region.GetTopLeft(), false );

    if ( c.mouse_state == Dragging )
        c.draw_zoom_rectangle( &dc );
}
void Canvas::DirectDrawer::finish() {
    if ( c.mouse_state == Dragging )
        c.draw_zoom_rectangle( &dc );
}

void Canvas::set_zoom(int zoom, wxPoint center)
{
    {
        /* Overpaint old image */
        wxClientDC dc(this);
        wxPen pen( *wxLIGHT_GREY );
        wxBrush brush( *wxLIGHT_GREY );
        dc.SetPen( pen );
        dc.SetBrush( brush );
        dc.DrawRectangle( canvas_coords(
            wxRect(0, 0, contents->GetWidth(), contents->GetHeight())) );
    }

    if ( center == wxDefaultPosition ) {
        int xl, yl, xs, ys, xr, yr;
        GetViewStart( &xl, &yl );
        GetScrollPixelsPerUnit( &xr, &yr );
        GetClientSize( &xs, &ys );
        center.x = xl*xr + xs / 2;
        center.y = yl*yr + ys / 2;
        center = image_coords( wxRect( center, wxSize(1,1) ) ).GetTopLeft();
    }

    if ( zoom < 0 ) {
        zoom_out_level = -zoom+1;
        zoom_in_level = 1;
    } else {
        zoom_out_level = 1;
        zoom_in_level = zoom+1;
    }
    if ( zcl )
        zcl->zoom_changed( zoom );
    wxSize new_size( contents->GetWidth(), contents->GetHeight() );
    SetVirtualSize( canvas_coords( new_size ).GetSize() );

    int xr, yr;
    GetScrollPixelsPerUnit( &xr, &yr );
    wxRect viewport = get_visible_region();

    /* Transform center to new scroll coordinates. */
    wxPoint top_left = 
            viewport.CenterIn(
                canvas_coords( wxRect( center, wxSize(1,1) ) )
            ).GetTopLeft();
    Scroll( top_left.x / xr, top_left.y / yr );

    wxClientDC dc(this);
    DoPrepareDC( dc );

    wxRect canvas_region = get_visible_region();
    wxBitmap bitmap = zoomed_bitmap_for_canvas_region( canvas_region );
    dc.DrawBitmap( bitmap, canvas_region.GetTopLeft() );

}

void Canvas::OnMouseDown( wxMouseEvent& event ) {
    drag_start.x = event.GetX();
    drag_start.y = event.GetY();
    drag_end = drag_start;
    mouse_state = Dragging;
}
void Canvas::OnMouseMotion( wxMouseEvent& event )
{
    if ( mouse_state == Dragging ) {
        wxClientDC dc( this );
        DoPrepareDC( dc );
        overdraw_rectangle( get_rect_by_corners( drag_start, drag_end ), &dc );
        drag_end.x = event.GetX();
        drag_end.y = event.GetY();
        draw_zoom_rectangle( &dc );
    }

    wxRect visible_region = get_visible_region();
    wxPoint mouse_in_canvas_space( 
        event.GetX() + visible_region.GetLeft(),
        event.GetY() + visible_region.GetTop() );
    wxPoint mouse_position = image_coords( mouse_in_canvas_space );
        
    if ( zcl && last_mouse_position != mouse_position )
        zcl->mouse_over_pixel( mouse_position );
    last_mouse_position = mouse_position;
}
void Canvas::OnMouseUp( wxMouseEvent& event ) 
{
    wxClientDC dc( this );
    DoPrepareDC( dc );
    overdraw_rectangle( get_rect_by_corners( drag_start, drag_end ), &dc );

    drag_end.x = event.GetX();
    drag_end.y = event.GetY();

    if ( zcl )
        zcl->drawn_rectangle(
            image_coords( get_rect_by_corners( drag_start, drag_end ) ) );
    mouse_state = Moving;
}

}
}
