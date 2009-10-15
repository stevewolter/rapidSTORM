#include "DisplayHandler.h"
#include <wx/wx.h>
#include <wx/evtloop.h>
#include <wx/dcbuffer.h>
#include <queue>
#include <stdexcept>
#include <map>

namespace dStorm {

struct Runnable {
    virtual void run() = 0;
    virtual void finish() {}
};
class WaitableRunnable : public Runnable {
    ost::Mutex mutex;
    bool did_run;
    ost::Condition cond;
  public:
    WaitableRunnable() : did_run(false), cond(mutex) {}
    void wait() {
        ost::MutexLock lock(mutex);
        while (!did_run) cond.wait();
    }
    void finish() { 
        ost::MutexLock lock(mutex);
        did_run = true;  
        cond.signal();
    }
};

wxColor makeColor( const dStorm::Color& c ) {
    return wxColor( c.r, c.g, c.b );
}

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
        assert( false );
    }

    void close() throw();

    void ensure_wx_initialization() {
        ost::MutexLock lock(mutex);
        while (!initialized) cond.wait();
    }
    void set_initialized() {
        ost::MutexLock lock(mutex);
        initialized = true;
        cond.signal();
    }

    DisplayHandler::WindowID
    make_image_window(
        const std::string& name,
        ViewportHandler& handler,
        const ResizeChange& init_size
    );
    void notify_of_vanished_data_source
        ( WindowID handle, bool close_immediately );
};

std::ostream& operator<<(std::ostream& o, const wxRect& rect ) {
    return (o << "(" << rect.GetLeft() << ", " << rect.GetTop() << ")+"
              << "(" << rect.GetRight() << ", " << rect.GetBottom() << ")");
}

class wxDisplayHandlerApp;
wxDisplayHandlerApp& wxGetApp();

class wxDisplayHandlerApp : public wxApp {
  public:
    class OutsideKnownWindow;
  private:
    ost::Mutex mutex;
    ost::Condition cond;
    std::queue< Runnable* > run_queue;

    std::map< DisplayHandler::WindowID, OutsideKnownWindow* >
        windows;
    std::map< OutsideKnownWindow*, DisplayHandler::WindowID >
        window_IDs;
    DisplayHandler::WindowID last_ID;

    std::auto_ptr<wxFrame> nevershow;

    DECLARE_EVENT_TABLE();

  public:
    wxDisplayHandlerApp() : cond(mutex), last_ID(0) {}

    class OutsideKnownWindow {
        DisplayHandler::WindowID id;
      public:
        OutsideKnownWindow(DisplayHandler::WindowID id)
            : id(id) 
        {
            wxDisplayHandlerApp& app = wxGetApp();
            ost::MutexLock lock( app.mutex );
            /* The get_new_ID call should have added a table entry
             * for us. If this table entry already vanished, the
             * window isn't needed anymore. */
            if ( app.windows.find( id ) != app.windows.end() ) {
                app.windows[id] = this;
                app.window_IDs.insert( std::make_pair( this, id ) );
            } else {
                assert( false );
            }
        }
        virtual ~OutsideKnownWindow() {
            wxDisplayHandlerApp& app = wxGetApp();
            ost::MutexLock lock( app.mutex );
            app.windows.erase( id );
            app.window_IDs.erase( this );
        }

        virtual void remove_connection( bool close_window ) = 0;

        static DisplayHandler::WindowID get_new_ID() {
            wxDisplayHandlerApp& app = wxGetApp();
            DisplayHandler::WindowID id = app.last_ID++;
            app.windows.insert( std::make_pair( id,
                static_cast<OutsideKnownWindow*>(NULL) ) );
            return id;
        }
        static bool need_detach( DisplayHandler::WindowID id ) {
            wxDisplayHandlerApp& app = wxGetApp();
            ost::MutexLock lock( app.mutex );
            if ( app.windows.find( id ) == app.windows.end() )
                /* Window was already detached. Do nothing */ ;
            else if ( app.windows[id] == NULL )
                /* Window was not yet created. Remove it to prevent
                 * creation. */
                app.windows.erase( id );
            else
                return true;
            return false;
        }
        static void detach( DisplayHandler::WindowID id, bool close ) {
            if ( !need_detach(id) ) return;

            class Deleter : public WaitableRunnable {
                DisplayHandler::WindowID id;
                bool close;
                public:
                Deleter( DisplayHandler::WindowID id,
                            bool close ) 
                    : id(id), close(close) {}
                void run() {
                    wxDisplayHandlerApp& app = wxGetApp();
                    ost::MutexLock lock( app.mutex );
                    if ( app.windows.find( id ) != app.windows.end() 
                            && app.windows[id] != NULL )
                        app.windows[id]->remove_connection(close);
                }
            };

            wxDisplayHandlerApp& app = wxGetApp();
            Deleter deleter(id, close);
            app.run_in_event_thread( deleter );
            deleter.wait();
        }
    };

    bool OnInit() {
        if ( !wxApp::OnInit() )
            return false;

        nevershow.reset( new wxFrame(NULL, wxID_ANY, _T("Nevershow")) );
        static_cast<wxDisplayHandler&>(DisplayHandler::getSingleton())
            .set_initialized();
        return true;
    }
    void OnIdle(wxIdleEvent& event) {
        ost::MutexLock lock(mutex);
        if ( !run_queue.empty() ) {
            while (!run_queue.empty()) {
                run_queue.front()->run();
                run_queue.front()->finish();
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
    }

    void close() { nevershow.reset( NULL ); }
};

void wxDisplayHandler::close() throw()
{
    wxGetApp().close();
}

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
    ImageCanvas( wxWindow *parent, wxWindowID id, const wxSize& init_size,
                 Color bg ) 
        : wxScrolledWindow(parent, id, wxDefaultPosition, wxDefaultSize),
          contents(init_size.GetWidth(), init_size.GetHeight(), false), 
          zoom_control(NULL), zoom_in_level( 1 ), zoom_out_level( 1 ),
          mouse_state( Moving )
    {
        SetScrollRate( 10, 10 );
        wxScrolledWindow::SetVirtualSize( init_size );
        SetBackgroundColour( *wxLIGHT_GREY );
        contents.SetRGB( wxRect( init_size ), bg.r, bg.g, bg.b );
    }

    class BufferedDrawer {
      protected:
        friend class ImageCanvas;
        ImageCanvas& c;

      public:
        BufferedDrawer( ImageCanvas& parent )
            : c( parent )
        {}
        inline void draw( int x, int y, const Color& c );
        void clear( const Color& c );
        void finish();
    };
    class DirectDrawer : public BufferedDrawer {
      protected:
        friend class ImageCanvas;
        wxClientDC dc;

      public:
        DirectDrawer( ImageCanvas& parent )
            : BufferedDrawer( parent ),
              dc( &c )
        {
            c.DoPrepareDC( dc );
        }

        inline void draw( int x, int y, const Color& c );
        void clear( const Color& c );
        void finish();
    };

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

void ImageCanvas::BufferedDrawer::draw( int x, int y, const Color& co ) {
    c.contents.SetRGB( x, y, co.r, co.g, co.b );
}

void ImageCanvas::DirectDrawer::draw( int x, int y, const Color& co ) {
    BufferedDrawer::draw( x, y, co );

    wxRect visible_region = c.get_visible_region();
    wxRect image_region( x, y, 1, 1 );
    wxRect canvas_region = c.canvas_coords( image_region ).Inflate( Overlap );
    if ( canvas_region.Intersects( visible_region ) ) {
        wxBitmap bitmap = c.zoomed_bitmap_for_canvas_region( canvas_region );
        dc.DrawBitmap( bitmap, canvas_region.GetTopLeft() );
    }
}

void ImageCanvas::BufferedDrawer::clear( const Color &color ) {
    unsigned char *ptr = c.contents.GetData();
    int size = 3 * c.contents.GetWidth() * c.contents.GetHeight();
    for (int i = 0; i < size; i += 3) {
        ptr[i] = color.r;
        ptr[i+1] = color.g;
        ptr[i+2] = color.b;
    }
}

void ImageCanvas::DirectDrawer::clear( const Color &color ) {
    wxPen pen( makeColor(color) );
    wxBrush brush( makeColor(color) );
    dc.SetPen( pen );
    dc.SetBrush( brush );
    dc.DrawRectangle( c.canvas_coords(
        wxRect(0, 0, c.contents.GetWidth(), c.contents.GetHeight())) );

    BufferedDrawer::clear( color );

    if ( c.mouse_state == Dragging )
        c.draw_zoom_rectangle( &dc );
}

void ImageCanvas::BufferedDrawer::finish() {
    wxClientDC dc(&c);
    c.DoPrepareDC( dc );

    wxRect canvas_region = c.get_visible_region();
    wxBitmap bitmap = c.zoomed_bitmap_for_canvas_region( canvas_region );
    dc.DrawBitmap( bitmap, canvas_region.GetTopLeft() );
}
void ImageCanvas::DirectDrawer::finish() {}

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

const wxChar *SI_prefixes[] = { _T("f"), _T("p"), _T("n"), _T("mu"), _T("m"),
                              _T(""), _T("k"), _T("M"), _T("G"), _T("T"),
                              _T("E") };

class ImageKey : public wxWindow {
    wxSize current_size;
    std::auto_ptr<wxBitmap> buffer;
    int num_keys, max_text_width, max_text_height;

    std::vector<Color> colors;
    std::vector<float> values;

    const int unlabelled_width, labelled_width;
    const int top_border, bottom_border, left_border;

    int key_distance, label_distance, line_height;

    wxPen background_pen;
    wxBrush background_brush;

    void compute_key_size() {
        int free_height = current_size.GetHeight() 
                            - top_border - bottom_border;
        if ( free_height < 0 ) { key_distance = 0; return; }

        key_distance = 1;
        line_height = 1;
        while ( free_height < line_height*num_keys/key_distance )
            key_distance += 1;
        while ( free_height > (line_height+1)*num_keys/key_distance )
            line_height += 1;
        label_distance = int(ceil(20.0 / line_height));
    }

    /* Center rect vertically with respect to other rect. Does what
     * wxRect::CentreIn should do, but doesn't in 2.8. */
    void center_rect_vertically( wxRect& centre, const wxRect& in ) {
        int cur_centre = (centre.GetBottom() + centre.GetTop())/2;
        int should = (in.GetBottom() + in.GetTop())/2;
        int shift = should - cur_centre;
        centre.y += shift;
    }

    void draw_key( int index, wxDC &dc ) {
        if ( key_distance == 0 ) 
            return;

        bool labelled = (index % label_distance) == 0;
        wxPen pen( makeColor( colors[index] ) );
        dc.SetPen( pen );
        wxBrush brush( makeColor( colors[index] ) );
        dc.SetBrush( brush );

        int line_length = ( labelled ) ? labelled_width : unlabelled_width;
        wxRect color_line( 
            left_border, 
            top_border+line_height*index/key_distance,
                           line_length, line_height );
        dc.DrawRectangle( color_line );

        if ( labelled ) {
            wxChar cbuffer[128];
            int log_10 = log10( values[index] );
            int postfix = int(floor( log_10 / 3 ));
            postfix = std::min( std::max( -5, postfix ), 5 );
            wxSnprintf(cbuffer, 128, wxT("%.*f %s"), 
                2 - (log_10 % 3),
                values[index] / pow(1000, postfix),
                SI_prefixes[postfix+5]);

            wxSize textSize = dc.GetTextExtent( wxString(cbuffer) );
            max_text_width = 
                std::max<int>( textSize.GetWidth()+20, max_text_width );
            max_text_height = 
                std::max<int>( textSize.GetHeight(), max_text_height );

            wxRect text_box( 
                wxPoint(2*left_border + labelled_width, color_line.GetTop()),
                textSize);
            center_rect_vertically( text_box, color_line );
            wxRect overdraw_box(
                text_box.GetLeft(), color_line.GetTop(),
                max_text_width, max_text_height);
            center_rect_vertically( overdraw_box, text_box );

            /* Overdraw old text */
            dc.SetPen( background_pen );
            dc.SetBrush( background_brush );
            dc.DrawRectangle( overdraw_box );

            dc.SetTextForeground(*wxWHITE);
            dc.SetTextBackground(*wxBLACK);
            dc.DrawText( wxString(cbuffer), text_box.GetTopLeft() );
        }
    }

  public:
    ImageKey( wxWindow* parent, wxSize size, int num_keys ) 
        : wxWindow( parent, wxID_ANY, wxDefaultPosition, size,
                    wxBORDER_SUNKEN ),
          current_size( size ),
          buffer( new wxBitmap(size.GetWidth(), size.GetHeight()) ),
          num_keys( num_keys ),
          max_text_width( 0 ), max_text_height( 0 ),
          colors( num_keys ),
          values( num_keys ),
          unlabelled_width(40),
          labelled_width(unlabelled_width+4),
          top_border( 30 ),
          bottom_border( 50 ),
          left_border(5),
          background_pen( *wxBLACK ),
          background_brush( *wxBLACK )
    {
        compute_key_size();
    }

    void draw_keys( const data_cpp::Vector<DisplayHandler::KeyChange>& kcs )
    {
        wxClientDC dc( this );
        wxBufferedDC buffer( &dc, *this->buffer );
        buffer.SetTextForeground( *wxWHITE );

        data_cpp::Vector<DisplayHandler::KeyChange>::const_iterator 
            i = kcs.begin(), end = kcs.end();

        for ( ; i != end; i++) {
            colors[i->index] = i->color;
            values[i->index] = i->value;

            draw_key( i->index, buffer );
        }
    }

    void OnPaint( wxPaintEvent& event ) {
        wxPaintDC dc(this);

        wxRegion updateRegions = GetUpdateRegion();
        for ( wxRegionIterator region( updateRegions ); region; ++region ){
            wxRect rect = region.GetRect();

            dc.DrawBitmap(buffer->GetSubBitmap( rect ), rect.GetTopLeft());
        }
    }
    void OnResize( wxSizeEvent& ) {
        wxSize size = GetClientSize();
        if ( size == current_size ) return;
        current_size = size;
        buffer.reset( new wxBitmap( size.GetWidth(), size.GetHeight() ) );
        wxClientDC base( this );
        wxBufferedDC dc( &base, *buffer );
        dc.SetPen( *wxWHITE_PEN );
        dc.SetBrush( *wxBLACK_BRUSH );
        dc.DrawRectangle( 0, 0, size.GetWidth(), size.GetHeight() );

        compute_key_size();
        for (int i = 0; i < num_keys; i++)
            draw_key( i, dc );
    }

    void resize( int new_number_of_keys ) {
    }
    DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(ImageKey, wxWindow)
    EVT_PAINT(ImageKey::OnPaint)
    EVT_SIZE(ImageKey::OnResize)
END_EVENT_TABLE()

class wxImageHandle 
: public wxFrame,
  public wxDisplayHandlerApp::OutsideKnownWindow
 {
  private:
    ImageCanvas* canvas;
    ImageKey* key;
    wxSlider *zoom;
    wxTimer timer;

    DisplayHandler::ViewportHandler* handler;

    DECLARE_EVENT_TABLE();

    void OnTimer(wxTimerEvent& event) {
        if ( !handler ) return;
        std::auto_ptr<DisplayHandler::Change>
            changes = handler->get_changes();

        commit_changes(*changes);

        event.Skip();
    }

    template <typename Drawer>
    void draw_image_window( const DisplayHandler::Change& changes ) {
        Drawer drawer( *canvas );

        if ( changes.do_clear )
            drawer.clear( changes.clear_image.background );

        data_cpp::Vector<DisplayHandler::PixelChange>::const_iterator 
            i = changes.change_pixels.begin(),
            end = changes.change_pixels.end();
        for ( ; i != end; i++)
            drawer.draw( i->x, i->y, i->color );

        drawer.finish();
    }

    void commit_changes(const DisplayHandler::Change& changes) {
        if ( changes.do_resize ) {
            const DisplayHandler::ResizeChange& r = changes.resize_image;
            canvas->resize( wxSize( r.width, r.height ) );
            key->resize( r.key_size );
        }

        if ( changes.change_pixels.size() < 1000 &&
             !changes.do_clear )
            draw_image_window<ImageCanvas::DirectDrawer>(changes);
        else
            draw_image_window<ImageCanvas::BufferedDrawer>(changes);

        key->draw_keys( changes.change_key );
    }

  public:
    wxImageHandle( DisplayHandler::WindowID ident,
                   const wxString& name, 
                   DisplayHandler::ViewportHandler& handler,
                   const DisplayHandler::ResizeChange& init_size)
    : wxFrame(NULL, wxID_ANY, name, wxDefaultPosition, wxSize(680, 680)),
      OutsideKnownWindow( ident ),
      timer(this, DISPLAY_TIMER),
      handler(&handler)
    {
        wxBoxSizer* outer_sizer = new wxBoxSizer( wxHORIZONTAL ),
                  * ver_sizer = new wxBoxSizer( wxVERTICAL ),
                  * hor_sizer = new wxBoxSizer( wxHORIZONTAL );
        canvas = new ImageCanvas(this, wxID_ANY, 
                    wxSize(init_size.width, init_size.height),
                    init_size.background);
        zoom = new wxSlider( this, wxID_ANY, 0, -16, 16, wxDefaultPosition,
                             wxDefaultSize, wxSL_AUTOTICKS | wxSL_LABELS );
        zoom->SetLineSize( 5 );
        zoom->SetTickFreq( 1, 1 );
        canvas->set_zoom_control( *zoom );

        key = new ImageKey( this, wxSize( 200, 0 ), init_size.key_size );

        ver_sizer->Add( canvas, wxSizerFlags(1).Expand() );
        hor_sizer->Add( new wxStaticText( this, wxID_ANY, _T("Zoom level") ), 
                        wxSizerFlags().Bottom().Border( wxALL, 10 ) );
        hor_sizer->Add( zoom, wxSizerFlags(1).Expand() );
        ver_sizer->Add( hor_sizer, wxSizerFlags().Expand() );
        outer_sizer->Add( ver_sizer, wxSizerFlags(1).Expand() );
        outer_sizer->Add( key, wxSizerFlags().Expand() );

        this->SetSizerAndFit( outer_sizer );

        timer.Start( 100 );
        Show( true );
    }

    void remove_connection(bool close_immediately) {
        timer.Stop();
        handler = NULL;
    }

    void OnClose(wxCloseEvent&) { this->Destroy(); }

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
    DisplayHandler::WindowID id;
    wxString name;
    wxDisplayHandler::ViewportHandler& handler;
    DisplayHandler::ResizeChange init_size;

  public:
    wxImageHandleCreator(
        const std::string& name,
        wxDisplayHandler::ViewportHandler& handler,
        const DisplayHandler::ResizeChange& init_size,
        DisplayHandler::WindowID& save_window_ID) 
        : id( wxDisplayHandlerApp::OutsideKnownWindow::get_new_ID() ),
          name( convert(name) ),
          handler( handler ),
          init_size( init_size )
    {
        save_window_ID = id;
        wxGetApp().run_in_event_thread( *this );
    }

    void run() {
        new wxImageHandle( id, name, handler, init_size );
    }
    void finish() {
        delete this;
    }
};

DisplayHandler::WindowID
wxDisplayHandler::make_image_window(
    const std::string& name,
    ViewportHandler& handler,
    const DisplayHandler::ResizeChange& init_size
)
{
    ensure_wx_initialization();
    WindowID id;
    new wxImageHandleCreator(name, handler, init_size, id);
    return id;
}

void 
wxDisplayHandler::notify_of_vanished_data_source
    ( WindowID handle, bool close_immediately ) 
{
    ensure_wx_initialization();
    wxDisplayHandlerApp::OutsideKnownWindow::detach
        ( handle, close_immediately );
}

}
