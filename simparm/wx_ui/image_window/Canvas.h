#ifndef DSTORM_DISPLAY_CANVAS_H
#define DSTORM_DISPLAY_CANVAS_H

#include <wx/wx.h>
#include "display/DataSource.h"

namespace simparm {
namespace wx_ui {
namespace image_window {

using dStorm::display::Color;
using dStorm::display::ImageChange;

class Canvas : public wxScrolledWindow {
  public:
    struct Listener {
        virtual void drawn_rectangle( wxRect rect ) = 0;
        virtual void zoom_changed( int to ) = 0;
        virtual void mouse_over_pixel( wxPoint, Color ) = 0;
    };
    void set_listener(Listener* listener);

  private:
    Listener* zcl;
    std::auto_ptr<wxImage> contents;

    const static int Overlap = 1;

    int zoom_in_level, zoom_out_level;

    enum Bound { Lower, Upper };
    inline int transform( int coords, int mult,
                          int div, Bound bound ); 
    inline int transform( bool mode, Bound bound,
                          int coord );

    inline const wxRect transformed_coords(
        const wxRect& conversion, bool to_image_coords );
    inline const wxPoint image_coords( const wxPoint& canvas_coords );
    inline const wxRect image_coords( const wxRect& canvas_coords );
    inline const wxPoint canvas_coords( const wxPoint& image_coords );
    inline const wxRect canvas_coords( const wxRect& image_coords );

    wxBitmap zoomed_bitmap_for_canvas_region( const wxRect& region );

    wxRect get_visible_region(); 

    enum MouseState { Moving, Dragging };
    MouseState mouse_state;
    wxPoint drag_start;
    wxPoint drag_end;
    wxPoint last_mouse_position;

    wxRect get_rect_by_corners( wxPoint a, wxPoint b );

    void draw_rectangle(wxRect rect, wxDC *dc);
    void overdraw_rectangle(wxRect rect, wxDC *dc);
    void draw_zoom_rectangle( wxDC *dc );

    void OnMouseDown( wxMouseEvent& event );
    void OnMouseMotion( wxMouseEvent& event );
    void OnMouseUp( wxMouseEvent& event );

  public:
    Canvas() {}
    Canvas( wxWindow *parent, wxWindowID id,
                 const wxSize& init_size );

    class BufferedDrawer;
    class DirectDrawer;

    void OnPaint( wxPaintEvent &event );
    void resize( const wxSize& new_size );

    void set_zoom( int zoom, wxPoint center = wxDefaultPosition ); 

    int getWidth() const 
        { return (contents.get()) ? contents->GetWidth() : 0; }
    int getHeight() const 
        { return (contents.get()) ? contents->GetHeight() : 0; }
    wxSize getSize() const 
        { return wxSize(getWidth(), getHeight()); }

    std::auto_ptr<ImageChange>
        getContents() const;

    /** @rect Coordinates in image pixels to zoom into view. */
    void zoom_to( wxRect rect );


    DECLARE_DYNAMIC_CLASS(ImageCanvas);
    DECLARE_EVENT_TABLE();
};

class Canvas::BufferedDrawer {
  protected:
    Canvas& c;

  public:
    BufferedDrawer( Canvas& parent )
        : c( parent )
    {}
    inline void draw( int x, int y, const Color& c );
    void clear( const Color& c );
    void finish();
};

class Canvas::DirectDrawer : public BufferedDrawer {
  protected:
    wxClientDC dc;

  public:
    DirectDrawer( Canvas& parent )
        : BufferedDrawer( parent ),
            dc( &c )
    {
        c.DoPrepareDC( dc );
    }

    void draw( int x, int y, const Color& c );
    void clear( const Color& c );
    void finish();
};

void Canvas::BufferedDrawer::draw( int x, int y, const Color& co ) {
    c.contents->SetRGB( x, y, co.red(), co.green(), co.blue() );
}

}
}
}

#endif
