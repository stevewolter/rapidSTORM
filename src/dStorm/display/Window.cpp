#include "Window.h"
#include "Canvas.h"
#include "ZoomSlider.h"
#include "Key.h"
#include "ScaleBar.h"

namespace dStorm {
namespace Display {

DECLARE_EVENT_TYPE(DISPLAY_TIMER, -1)
DEFINE_EVENT_TYPE(DISPLAY_TIMER)

wxString std_to_wx_string( const std::string& a ) {
    return wxString( a.c_str(), wxMBConvLibc() );
}

Window::Window(
    const Manager::WindowProperties& props,
    DataSource* data_source,
    wxManager::WindowHandle *my_handle
)
: wxFrame(NULL, wxID_ANY, std_to_wx_string( props.name ),
          wxDefaultPosition, wxSize(680, 680)),
    timer(this, DISPLAY_TIMER),
    source(data_source),
    handle( my_handle )
{
    const ResizeChange& init_size = props.initial_size;

    wxBoxSizer* outer_sizer = new wxBoxSizer( wxHORIZONTAL ),
                * ver_sizer1 = new wxBoxSizer( wxVERTICAL ),
                * ver_sizer2 = new wxBoxSizer( wxVERTICAL ),
                * hor_sizer = new wxBoxSizer( wxHORIZONTAL );
    canvas = new Canvas(this, wxID_ANY, 
                wxSize(init_size.width, init_size.height));
    zoom = new ZoomSlider( this, *canvas );
    zoom->set_zoom_listener( *this );

    wxClientDC dc(this);
    wxSize size = dc.GetTextExtent( wxT("0123456789.") );
    key = new Key( this, wxSize( 100+size.GetWidth()*2 / 11, 0 ),
                   init_size.key_size );

    scale_bar = new ScaleBar(this, wxSize(150, 30));
    scale_bar->set_pixel_size( init_size.pixel_size );
    scale_bar->set_zoom_factor( 1 );

    ver_sizer1->Add( canvas, wxSizerFlags(1).Expand() );
    hor_sizer->Add( new wxStaticText( this, wxID_ANY, wxT("Zoom level") ), 
                    wxSizerFlags().Center().Border( wxALL, 10 ) );
    hor_sizer->Add( zoom, wxSizerFlags(1).Expand() );
    hor_sizer->Add( new wxStaticText( this, wxID_ANY, wxT("Scale bar")),
                    wxSizerFlags().Center().Border( wxALL, 10 ) );
    hor_sizer->Add( scale_bar, wxSizerFlags().Expand().Border( wxALL, 10 ) );
    ver_sizer1->Add( hor_sizer, wxSizerFlags().Expand() );
    ver_sizer2->Add( new wxStaticText( this, wxID_ANY, wxT("Key (in ADC)")),
                    wxSizerFlags().Center().Border( wxALL, 10 ) );
    ver_sizer2->Add( key, wxSizerFlags(1).Expand() );
    outer_sizer->Add( ver_sizer1, wxSizerFlags(1).Expand() );
    outer_sizer->Add( ver_sizer2, wxSizerFlags().Expand() );

    this->SetSizerAndFit( outer_sizer );

    timer.Start( 100 );
    Show( true );
}

Window::~Window()
{
    if ( handle != NULL ) {
        wxManager::disassociate_window( this, handle );
        handle = NULL;
    }
}

void Window::OnTimer(wxTimerEvent& event) {
    if ( !source ) return;
    std::auto_ptr<Change> changes = source->get_changes();

    commit_changes(*changes);

    event.Skip();
}

template <typename Drawer>
void Window::draw_image_window( const Change& changes ) {
    Drawer drawer( *canvas );

    if ( changes.do_clear )
        drawer.clear( changes.clear_image.background );

    data_cpp::VectorList<PixelChange>::const_iterator 
        i = changes.change_pixels.begin(),
        end = changes.change_pixels.end();
    for ( ; i != end; i++)
        drawer.draw( i->x, i->y, i->color );

    drawer.finish();
}

void Window::commit_changes(const Change& changes)
{
    if ( changes.do_resize ) {
        const ResizeChange& r = changes.resize_image;
        canvas->resize( wxSize( r.width, r.height ) );
        key->resize( r.key_size );
        scale_bar->set_pixel_size( r.pixel_size );
    }

    if ( changes.change_pixels.size() < 1000 &&
            !changes.do_clear )
        draw_image_window<Canvas::DirectDrawer>(changes);
    else
        draw_image_window<Canvas::BufferedDrawer>(changes);

    key->draw_keys( changes.change_key );
}

void Window::remove_data_source() {
    timer.Stop();
    source = NULL;
}

void Window::zoom_changed( int to ) {
    scale_bar->set_zoom_factor( 
        ( to >= 0 ) ? (1.0 / (to + 1)) : (-to+1) );
}

BEGIN_EVENT_TABLE(Window, wxFrame)
    EVT_TIMER(DISPLAY_TIMER, Window::OnTimer)
END_EVENT_TABLE()

}
}
