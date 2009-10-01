#include "DisplayHandler.h"
#include <wx/wx.h>
#include <wx/evtloop.h>
#include <wx/dcbuffer.h>
#include <queue>
#include <stdexcept>

namespace dStorm {

struct Runnable {
    virtual void run() = 0;
};

class wxDisplayHandler : public DisplayHandler {
  private:
    bool initialized;
    ost::Mutex mutex;
    ost::Condition cond;

  public:
    wxDisplayHandler()
        : initialized(false), cond(mutex)
    {
        detach();
    }
    void run() throw() {
        int argc = 0;
        wxEntry(argc, (wxChar**)NULL);
    }

    void ensure_wx_initialization() {
        ost::MutexLock lock(mutex);
        while (!initialized) cond.wait();
    }
    void set_initialized() {
        ost::MutexLock lock(mutex);
        initialized = true;
        cond.signal();
    }

    std::auto_ptr<DisplayHandler::ImageHandle> 
    makeImageWindow(
        const std::string& name,
        ViewportHandler& handler
    );
    void returnImageWindow
        ( std::auto_ptr<ImageHandle> handle, bool close_immediately );
};

std::ostream& operator<<(std::ostream& o, const wxRect& rect ) {
    return (o << "(" << rect.GetLeft() << ", " << rect.GetTop() << ")+"
              << "(" << rect.GetRight() << ", " << rect.GetBottom() << ")");
}

class wxDisplayHandlerApp : public wxApp {
    ost::Mutex mutex;
    ost::Condition cond;
    std::queue< Runnable* > run_queue;

    DECLARE_EVENT_TABLE();

  public:
    wxDisplayHandlerApp() : cond(mutex) {}

    bool OnInit() {
        if ( !wxApp::OnInit() )
            return false;

        static_cast<wxDisplayHandler&>(DisplayHandler::getSingleton())
            .set_initialized();
        return true;
    }
    void OnIdle(wxIdleEvent& event) {
        ost::MutexLock lock(mutex);
        if ( !run_queue.empty() ) {
            while (!run_queue.empty()) {
                run_queue.front()->run();
                run_queue.pop();
            }
            cond.signal();
        }
        event.Skip();
    }

    void run_in_event_thread( Runnable& thread ) {
        ost::MutexLock lock(mutex);
        run_queue.push( &thread );
        wxWakeUpIdle();
        while (!run_queue.empty())
            cond.wait();
    }
};

BEGIN_EVENT_TABLE(wxDisplayHandlerApp, wxApp)
    EVT_IDLE(wxDisplayHandlerApp::OnIdle)
END_EVENT_TABLE()

IMPLEMENT_APP_NO_MAIN( wxDisplayHandlerApp );

DECLARE_EVENT_TYPE(DISPLAY_TIMER, -1)
DEFINE_EVENT_TYPE(DISPLAY_TIMER)

class ImageCanvas : public wxScrolledWindow {
    wxImage contents;

    wxSlider *zoom_control;
    const static int Overlap = 1;

    int zoom_in_level, zoom_out_level;

    enum Bound { Lower, Upper };
    int transform( int coords, int mult, int div, Bound bound ) {
        int rv = coords * mult / div;
        if ( mult > 1 && bound == Upper )
            rv += mult - 1;
        return rv;
    }
    int transform( bool mode, Bound bound, int coord ) { 
        return transform( coord, 
            ( mode ) ? zoom_out_level : zoom_in_level,
            ( mode ) ? zoom_in_level : zoom_out_level,
            bound );
    }

    wxRect transformed_coords( const wxRect& conversion, 
        bool to_image_coords )
    {
        int left = transform( to_image_coords, Lower, conversion.x );
        int top = transform( to_image_coords, Lower, conversion.y );
        int right = transform( to_image_coords, Upper, conversion.GetRight() ),
            bottom = transform( to_image_coords, Upper, conversion.GetBottom() );
        return wxRect( left, top, right - left + 1, bottom - top + 1 );
    }
    wxRect image_coords( const wxRect& canvas_coords ) {
        return transformed_coords( canvas_coords, true );
    }
    wxRect canvas_coords( const wxRect& image_coords ) {
        return transformed_coords( image_coords, false );
    }

    wxBitmap zoomed_bitmap_for_canvas_region( const wxRect& region );

    wxRect get_visible_region() {
        int xl, yl, xs, ys, xr, yr;
        GetScrollPixelsPerUnit( &xr, &yr );
        GetClientSize( &xs, &ys );
        GetViewStart( &xl, &yl );
        return wxRect( xl * xr, yl * yr, xs, ys );
    }

    enum MouseState { Moving, Dragging };
    MouseState mouse_state;
    wxPoint drag_start;
    wxPoint drag_end;

    wxRect get_rect_by_corners( wxPoint a, wxPoint b ) {
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

    void draw_rectangle(wxRect rect, wxDC *dc) {
        dc->SetPen( *wxMEDIUM_GREY_PEN );
        dc->SetBrush( *wxTRANSPARENT_BRUSH );
        dc->DrawRectangle( rect );
    }
    void overdraw_rectangle(wxRect rect, wxDC *dc) {
        wxRect edges[4];
        wxSize hor(rect.GetWidth(), 1), ver(1, rect.GetHeight());
        edges[0] = wxRect( rect.GetTopLeft(), hor );
        edges[1] = wxRect( rect.GetTopLeft(), ver );
        edges[2] = wxRect( rect.GetTopRight(), ver );
        edges[3] = wxRect( rect.GetBottomLeft(), hor );

        for (int i = 0; i < 4; i++)
            dc->DrawBitmap( zoomed_bitmap_for_canvas_region( edges[i] ),
                        edges[i].GetTopLeft() );
    }
    void draw_zoom_rectangle( wxDC *dc ) {
        draw_rectangle( 
            get_rect_by_corners( drag_start, drag_end ), dc );
    }

    /** @rect Coordinates in image pixels to zoom into view. */
    void zoom_to( wxRect rect ) {
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

  public:
    ImageCanvas() {}
    ImageCanvas( wxWindow *parent, wxWindowID id, const wxSize& init_size ) 
        : wxScrolledWindow(parent, id, wxDefaultPosition, wxDefaultSize),
          contents(0, 0), 
          zoom_control(NULL), zoom_in_level( 1 ), zoom_out_level( 1 ),
          mouse_state( Moving )
    {
        SetScrollRate( 10, 10 );
        wxScrolledWindow::SetVirtualSize( init_size );
        SetBackgroundColour( *wxLIGHT_GREY );
    }

    class Drawer {
      private:
        friend class ImageCanvas;
        ImageCanvas& c;
        wxClientDC dc;

        Drawer( ImageCanvas& parent )
            : c( parent ),
              dc( &c )
        {
            c.DoPrepareDC( dc );
        }
      public:
        void draw( int x, int y, uint8_t r, uint8_t g, uint8_t b );
        void clear( uint8_t r, uint8_t g, uint8_t b );
    };
    std::auto_ptr<Drawer> getDrawer()
        { return std::auto_ptr<Drawer>( new Drawer(*this) ); }

    void OnPaint( wxPaintEvent &event );
    void resize( const wxSize& new_size );

    void set_zoom_control( wxSlider& control ) {
        zoom_control = &control;
    }
    void set_zoom( int zoom, wxPoint center = wxDefaultPosition ); 

    void OnMouseDown( wxMouseEvent& event ) {
        drag_start.x = event.GetX();
        drag_start.y = event.GetY();
        drag_end = drag_start;
        mouse_state = Dragging;
    }
    void OnMouseMotion( wxMouseEvent& event ) {
        if ( mouse_state == Dragging ) {
            wxClientDC dc( this );
            DoPrepareDC( dc );
            overdraw_rectangle( get_rect_by_corners( drag_start, drag_end ), &dc );
            drag_end.x = event.GetX();
            drag_end.y = event.GetY();
            draw_zoom_rectangle( &dc );
        }
    }
    void OnMouseUp( wxMouseEvent& event ) {
        drag_end.x = event.GetX();
        drag_end.y = event.GetY();
        zoom_to( image_coords(
            get_rect_by_corners( drag_start, drag_end ) ) );
        mouse_state = Moving;
    }

    DECLARE_DYNAMIC_CLASS(ImageCanvas);
    DECLARE_EVENT_TABLE();
};

IMPLEMENT_DYNAMIC_CLASS(ImageCanvas, wxScrolledWindow);

BEGIN_EVENT_TABLE(ImageCanvas, wxScrolledWindow)
    EVT_PAINT(ImageCanvas::OnPaint)
    EVT_LEFT_DOWN(ImageCanvas::OnMouseDown)
    EVT_LEFT_UP(ImageCanvas::OnMouseUp)
    EVT_MOTION(ImageCanvas::OnMouseMotion)
END_EVENT_TABLE()

void ImageCanvas::OnPaint( wxPaintEvent & ) {
    wxPaintDC dc(this);
    if ( contents.GetWidth() <= 0 ) return;
    wxScrolledWindow::PrepareDC(dc);

    wxRegion updateRegions = GetUpdateRegion();
    for ( wxRegionIterator region( updateRegions ); region; ++region ) {
        wxRect rect = region.GetRect();
        CalcUnscrolledPosition( rect.x, rect.y, &rect.x, &rect.y );

        /* Find the actual image part of the update region. */
        wxRect image_area = wxRect( wxSize(contents.GetWidth(),
                                           contents.GetHeight()) );
        rect.Intersect( canvas_coords( image_area ) );
        dc.DrawBitmap( zoomed_bitmap_for_canvas_region( rect ),
                       rect.GetTopLeft() );
    }
}

wxBitmap ImageCanvas::zoomed_bitmap_for_canvas_region
    ( const wxRect& unclipped_rect )
{
    wxRect image_area = wxRect( wxSize(contents.GetWidth(),
                                       contents.GetHeight()) );
    wxRect rect = unclipped_rect.Intersect( canvas_coords( image_area ) );

    wxRect subimage = image_coords( rect ), subcanvas = rect;
    wxRect smoothing_region_in_image = subimage,
            smoothing_region_in_canvas;

    smoothing_region_in_image.Inflate( Overlap * zoom_out_level )
        .Intersect( image_area );

    smoothing_region_in_canvas = 
        canvas_coords( smoothing_region_in_image );

    wxImage toScale = contents.GetSubImage( smoothing_region_in_image );
    toScale.Rescale( smoothing_region_in_canvas.GetWidth(), 
                        smoothing_region_in_canvas.GetHeight() );
    wxRect update_region_in_enlarged( rect );
    update_region_in_enlarged.Offset( 
        - smoothing_region_in_canvas.GetTopLeft() );
    wxImage toDraw = toScale.GetSubImage( update_region_in_enlarged );
    
    return wxBitmap( toDraw );
}

void ImageCanvas::resize( const wxSize& new_size ) {
    contents.Create( new_size.GetWidth(), new_size.GetHeight() );
    wxScrolledWindow::SetVirtualSize( canvas_coords( new_size ).GetSize() );

    zoom_to( canvas_coords( new_size ) );
}

void ImageCanvas::Drawer::draw( int x, int y, uint8_t r, uint8_t g, uint8_t b ) {
    c.contents.SetRGB( x, y, r, g, b );

    wxRect visible_region = c.get_visible_region();
    wxRect image_region( x, y, 1, 1 );
    wxRect canvas_region = c.canvas_coords( image_region ).Inflate( Overlap );
    if ( canvas_region.Intersects( visible_region ) ) {
        wxBitmap bitmap = c.zoomed_bitmap_for_canvas_region( canvas_region );
        dc.DrawBitmap( bitmap, canvas_region.GetTopLeft() );
    }
}

void ImageCanvas::Drawer::clear( uint8_t r, uint8_t g, uint8_t b ) {
    wxPen pen( wxColour(r,g,b) );
    wxBrush brush( wxColour(r,g,b) );
    dc.SetPen( pen );
    dc.SetBrush( brush );
    dc.DrawRectangle( c.canvas_coords(
        wxRect(0, 0, c.contents.GetWidth(), c.contents.GetHeight())) );
    unsigned char *ptr = c.contents.GetData();
    int size = 3 * c.contents.GetWidth() * c.contents.GetHeight();
    for (int i = 0; i < size; i += 3) {
        ptr[i] = r;
        ptr[i+1] = g;
        ptr[i+2] = b;
    }

    if ( c.mouse_state == Dragging )
        c.draw_zoom_rectangle( &dc );
}

void ImageCanvas::set_zoom(int zoom, wxPoint center)
{
    {
        /* Overpaint old image */
        wxClientDC dc(this);
        wxPen pen( *wxLIGHT_GREY );
        wxBrush brush( *wxLIGHT_GREY );
        dc.SetPen( pen );
        dc.SetBrush( brush );
        dc.DrawRectangle( canvas_coords(
            wxRect(0, 0, contents.GetWidth(), contents.GetHeight())) );
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
    if ( zoom_control && zoom_control->GetValue() != zoom )
        zoom_control->SetValue( zoom );
    wxSize new_size( contents.GetWidth(), contents.GetHeight() );
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

class wxImageHandle : public DisplayHandler::ImageHandle, public wxFrame {
  private:
    ImageCanvas* canvas;
    wxSlider *zoom;
    wxTimer timer;

    ost::Mutex handler_mutex;
    DisplayHandler::ViewportHandler* handler;

    bool resize_on_timer;
    int width, height;
    bool clear_on_timer;
    uint8_t r, g, b;

    DECLARE_EVENT_TABLE();

    void OnTimer(wxTimerEvent& event) {
        ost::MutexLock handler_mutex_lock( handler_mutex );
        if ( !handler || !size_was_set ) return;
        handler->clean();

        ost::MutexLock lock( handler->mutex );
        commit_changes();

        event.Skip();
    }

    void commit_changes() {
        if ( !resize_on_timer &&
             !clear_on_timer && pending_changes.size() == 0 ) return;

        if ( resize_on_timer ) {
            canvas->resize( wxSize( width, height ) );
            resize_on_timer = false;
        }

        std::auto_ptr<ImageCanvas::Drawer> drawer
            = canvas->getDrawer();

        if ( clear_on_timer ) {
            clear_on_timer = false;
            drawer->clear( r, g, b );
        }

        data_cpp::Vector<Pixel>::iterator 
            i = pending_changes.begin(),
            end = pending_changes.end();
        for ( ; i != end; i++) {
            Pixel& p = *i;
            drawer->draw( p.x, p.y, p.r, p.g, p.b );
        }
        pending_changes.clear();
    }

    bool size_was_set;

  public:
    wxImageHandle( const wxString& name, 
                   DisplayHandler::ViewportHandler& handler )
    : wxFrame(NULL, wxID_ANY, name, wxDefaultPosition, wxSize(680, 680)),
      timer(this, DISPLAY_TIMER),
      handler(&handler),
      clear_on_timer( false ),
      size_was_set( false )
    {
        wxBoxSizer* ver_sizer = new wxBoxSizer( wxVERTICAL ),
                  * hor_sizer = new wxBoxSizer( wxHORIZONTAL );
        canvas = new ImageCanvas(this, wxID_ANY, wxSize(640, 640));
        zoom = new wxSlider( this, wxID_ANY, 0, -16, 16, wxDefaultPosition,
                             wxDefaultSize, wxSL_AUTOTICKS | wxSL_LABELS );
        zoom->SetLineSize( 5 );
        zoom->SetTickFreq( 1, 1 );
        canvas->set_zoom_control( *zoom );

        ver_sizer->Add( canvas, wxSizerFlags(1).Expand() );
        hor_sizer->Add( new wxStaticText( this, wxID_ANY, _T("Zoom level") ), 
                        wxSizerFlags().Bottom().Border( wxALL, 10 ) );
        hor_sizer->Add( zoom, wxSizerFlags(1).Expand() );
        ver_sizer->Add( hor_sizer, wxSizerFlags().Expand() );
        this->SetSizerAndFit( ver_sizer );

        timer.Start( 100 );
        Show( true );
    }
    ~wxImageHandle() {
        timer.Stop();
    }

    void setSize( int width, int height )
    {
        ost::MutexLock handler_mutex_lock( handler_mutex );
        resize_on_timer = true;
        this->width = width;
        this->height = height;
        size_was_set = true;
    }

    void clear( uint8_t r, uint8_t g, uint8_t b ) {
        clear_on_timer = true;
        this->r = r; this->g = g; this->b = b;
        pending_changes.clear();
    }

    void delete_when_finished() {
        timer.Stop();
        ost::MutexLock handler_mutex_lock( handler_mutex );
        commit_changes();
        handler = NULL;
    }

    void OnClose(wxCloseEvent&) {
        ost::MutexLock handler_mutex_lock( handler_mutex );
        if ( handler != NULL ) {
            ost::MutexLock lock( handler->mutex );
            std::auto_ptr<ImageHandle> me = handler->detach_from_window();
            /* Will be destroyed by this->Destroy(). */
            me.release();
            handler = NULL;
        }
        this->Destroy();
    }

    void OnZoomChange( wxScrollEvent& event ) {
        canvas->set_zoom( event.GetPosition() );
        event.Skip();
    }
};

BEGIN_EVENT_TABLE(wxImageHandle, wxFrame)
    EVT_TIMER(DISPLAY_TIMER, wxImageHandle::OnTimer)
    EVT_CLOSE( wxImageHandle::OnClose )
    EVT_SCROLL_CHANGED( wxImageHandle::OnZoomChange ) 
    EVT_SCROLL_THUMBTRACK( wxImageHandle::OnZoomChange ) 
END_EVENT_TABLE()

DisplayHandler& DisplayHandler::getSingleton() {
    static DisplayHandler* handler = new wxDisplayHandler();
    return *handler;
}

wxString convert( const std::string& a ) {
    return wxString( a.c_str(), wxMBConvLibc() );
}

class wxImageHandleCreator : public Runnable
{
    wxString name;
    wxDisplayHandler::ViewportHandler& handler;
    std::auto_ptr<DisplayHandler::ImageHandle> rv;
  public:
    wxImageHandleCreator(
        const std::string& name,
        wxDisplayHandler::ViewportHandler& handler) 
        : name( convert(name) ),
          handler( handler )
    {
        wxGetApp().run_in_event_thread( *this );
    }

    void run() {
        rv.reset( new wxImageHandle( name, handler ) );
    }

    std::auto_ptr<DisplayHandler::ImageHandle> get() { return rv; }
};

class wxImageHandleDestructor : public Runnable {
    std::auto_ptr<DisplayHandler::ImageHandle> handle;
    bool immediate_close;
  public:
    wxImageHandleDestructor( 
        std::auto_ptr<DisplayHandler::ImageHandle> handle,
        bool immediate_close
    ) : handle(handle), immediate_close(immediate_close)
    {
        wxGetApp().run_in_event_thread( *this );
    }

    void run() {
        if ( immediate_close )
            handle.reset( NULL );
        else {
            wxImageHandle* ih = 
                static_cast<wxImageHandle*>( handle.release() );
            ih->delete_when_finished();
        }

        delete this;
    }
};

std::auto_ptr<DisplayHandler::ImageHandle> 
wxDisplayHandler::makeImageWindow(
    const std::string& name,
    ViewportHandler& handler
)
{
    ensure_wx_initialization();
    return wxImageHandleCreator(name, handler).get();
}

void 
wxDisplayHandler::returnImageWindow
    ( std::auto_ptr<ImageHandle> handle, bool close_immediately ) 
{
    ensure_wx_initialization();
    new wxImageHandleDestructor( handle, close_immediately );
}

}
