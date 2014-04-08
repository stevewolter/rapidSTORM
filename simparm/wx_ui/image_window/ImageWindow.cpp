#include <stdlib.h>
#include "simparm/wx_ui/image_window/ImageWindow.h"
#include "simparm/wx_ui/image_window/Canvas.h"
#include "simparm/wx_ui/image_window/ZoomSlider.h"
#include "simparm/wx_ui/image_window/Key.h"
#include "simparm/wx_ui/image_window/ScaleBar.h"
#include <sstream>
#include "simparm/wx_ui/image_window/SizeConvert.h"

#include "debug.h"

namespace simparm {
namespace wx_ui {
namespace image_window {

using dStorm::display::PixelChange;

wxString std_to_wx_string( const std::string& a ) {
    return wxString( a.c_str(), wxMBConvLibc() );
}

Window::Window(
    const dStorm::display::WindowProperties& props,
    boost::shared_ptr< SharedDataSource > data_source
)
: wxFrame(NULL, wxID_ANY, std_to_wx_string( props.name ),
          wxDefaultPosition, wxSize(780, 780)),
    timer( this, wxID_ANY ),
    data_source(data_source),
    close_on_completion( props.flags.get_close_window_on_unregister() ),
    notify_for_zoom( props.flags.get_notice_drawn_rectangle() ),
    has_3d( false )
{
    int display_timer = 100;
    if ( getenv("RAPIDSTORM_DISPLAY_FREQUENCY") ) {
        float f = atof(getenv("RAPIDSTORM_DISPLAY_FREQUENCY"));
        if ( f > 0 )
            display_timer = 1000 / f;
    }
    timer.Start( display_timer );

    const ResizeChange& init_size = props.initial_size;
    SetBackgroundColour(
        wxSystemSettings::GetColour(wxSYS_COLOUR_MENU));

    wxBoxSizer* outer_sizer = new wxBoxSizer( wxHORIZONTAL ),
                * ver_sizer1 = new wxBoxSizer( wxVERTICAL ),
                * ver_sizer2 = new wxBoxSizer( wxVERTICAL ),
                * hor_sizer = new wxBoxSizer( wxHORIZONTAL ),
                * key_sizer = new wxBoxSizer( wxHORIZONTAL );
    has_3d = init_size.size.z() > 1 * camera::pixel;
    wxSize initSize = mkWxSize(init_size.size);
    canvas = new Canvas(this, wxID_ANY, initSize);
    zoom = new ZoomSlider( this, *canvas );
    zoom->set_zoom_listener( *this );

    position_label = new wxStaticText(this, wxID_ANY, wxT(""));

    wxClientDC dc(this);
    wxSize size = dc.GetTextExtent( wxT("0123456789.") );
    for (unsigned int i = 0; i < init_size.keys.size(); i++) {
        keys.push_back( new Key( i, this, wxSize( 100+size.GetWidth()*2 / 11, 0 ),
                                init_size.keys[i], data_source ) );
    }

    scale_bar = new ScaleBar(this, wxSize(150, 30));
    scale_bar->set_pixel_size( init_size.pixel_sizes[0] );
    scale_bar->set_zoom_factor( 1 );

    ver_sizer1->Add( canvas, wxSizerFlags(1).Expand() );
    hor_sizer->Add( new wxStaticText( this, wxID_ANY, wxT("Zoom level") ), 
                    wxSizerFlags().Center().Border( wxALL, 10 ) );
    hor_sizer->Add( zoom, wxSizerFlags(1).Expand() );
    hor_sizer->Add( new wxStaticText( this, wxID_ANY, wxT("Scale bar")),
                    wxSizerFlags().Center().Border( wxALL, 10 ) );
    hor_sizer->Add( scale_bar, wxSizerFlags().Expand().Border( wxALL, 10 ) );
    ver_sizer1->Add( hor_sizer, wxSizerFlags().Expand() );
    for (Keys::const_iterator i = keys.begin(); i != keys.end(); ++i) {
        key_sizer->Add( (*i)->getBox(), wxSizerFlags(1).Expand() );
    }
    ver_sizer2->Add( key_sizer, wxSizerFlags(1).Expand() );
    ver_sizer2->Add( position_label, wxSizerFlags() );
    outer_sizer->Add( ver_sizer1, wxSizerFlags(1).Expand() );
    outer_sizer->Add( ver_sizer2, wxSizerFlags().Expand() );

    this->SetSizer( outer_sizer );

    Show( true );

    float ratio = std::min( 600.0f / initSize.GetWidth(), 600.0f / initSize.GetHeight() );
    int new_zoom = ( ratio < 1 ) 
        ? 1 - int( ceil(1/ratio) )
        : 1 + int( floor(ratio) );
    canvas->set_zoom(new_zoom);

    this->Raise();
}

void Window::UserClosedWindow(wxCloseEvent& e) {
    data_source->notice_closed_data_window();

    if ( ! data_source->notify_of_closed_window_before_disconnect() ) {
        this->Destroy();
    } else if ( e.CanVeto() ) {
        e.Veto();
    } else {
        std::cerr << "wxWidgets sent an unvetoable window close request, rapidSTORM can't handle this gracefully" << std::endl;
        std::abort();
    }
}

Window::~Window()
{
}

void Window::update_image() {
    std::auto_ptr<Change> changes = data_source->get_changes();
    if ( changes.get() )
        commit_changes(*changes);
    else
        notice_that_source_has_disappeared();
}

template <typename Drawer>
void Window::draw_image_window( const Change& changes ) {
    Drawer drawer( *canvas );

    if ( changes.do_clear ) {
        background = changes.clear_image.background;
        drawer.clear( background );
    }

    int width = canvas->getWidth();
    int height = canvas->getHeight();
    if ( changes.do_change_image ) {
        dStorm::display::Image::Position pos;
        pos.fill(0);
        for ( int y = 0; y < height; y++ ) {
            pos.y() = y * camera::pixel;
            for ( int x = 0; x < width; x++) {
                pos.x() = x * camera::pixel;
                drawer.draw(x, y, changes.image_change.new_image(pos));
            }
        }
    }
            
    std::vector<PixelChange>::const_iterator 
        i = changes.change_pixels.begin(),
        end = changes.change_pixels.end();
    for ( ; i != end; i++)
        if ( i->z() == 0 * camera::pixel )
            drawer.draw( i->x() / camera::pixel, i->y() / camera::pixel, i->color );

    drawer.finish();
}

void Window::commit_changes(const Change& changes)
{
    DEBUG("Committing changes");
    if ( changes.do_resize ) {
        DEBUG("Resize");
        const ResizeChange& r = changes.resize_image;
        canvas->resize( mkWxSize( r.size ) );
        has_3d = r.size.z() > 1 * camera::pixel;
        assert( keys.size() >= r.keys.size() );
        for (unsigned int i = 0; i < r.keys.size(); ++i) {
            keys[i]->resize( r.keys[i] );
        }
        scale_bar->set_pixel_size( r.pixel_sizes[0] );
    }

    if ( changes.change_pixels.size() < 1000 &&
            !changes.do_clear && !changes.do_change_image )
        draw_image_window<Canvas::DirectDrawer>(changes);
    else {
        draw_image_window<Canvas::BufferedDrawer>(changes);
    }

    DEBUG("Committing key changes for " << changes.changed_keys.size() << " keys into " << keys.size() << " windows");
    assert( changes.changed_keys.size() <= keys.size() );
    for (unsigned int i = 0; i < changes.changed_keys.size(); ++i)
        keys[i]->draw_keys( changes.changed_keys[i] );
}

/** Clean up after the DataSource object has been removed. This method is only callable from the GUI thread. */
void Window::notice_that_source_has_disappeared() {
    data_source->disconnect();

    for (Keys::iterator i = keys.begin(); i != keys.end(); ++i) 
        (*i)->freeze_limit_changers();
        
    if ( close_on_completion || data_source->notify_of_closed_window_before_disconnect() ) {
        DEBUG("Destroying window");
        this->Destroy();
    }
}

void Window::drawn_rectangle( wxRect rect ) {
    DEBUG("Drawn rectangle");
    if ( notify_for_zoom ) {
        DEBUG("Checking source");
        data_source->notice_drawn_rectangle( 
            rect.GetLeft(), rect.GetRight(), 
            rect.GetTop(), rect.GetBottom() );
        update_image();
    } else {
        canvas->zoom_to( rect );
    }
}

void Window::mouse_over_pixel( wxPoint point, Color color ) {
    std::stringstream label_text;
    label_text << "(" << point.x << ", " << point.y << ")";
    position_label->SetLabel( std_to_wx_string( label_text.str() ) );

    dStorm::display::Image::Position pos = dStorm::display::Image::Position::Zero();
    pos.x() = point.x * camera::pixel;
    pos.y() = point.y * camera::pixel;
    dStorm::display::DataSource::PixelInfo info( pos, color );
    std::vector<float> key_values( keys.size(), std::numeric_limits<float>::quiet_NaN() );
    data_source->look_up_key_values( info, key_values );
    for (size_t i = 0; i < keys.size(); ++i) 
        keys[i]->cursor_value( info, key_values[i] );
}

void Window::zoom_changed( int to ) {
    scale_bar->set_zoom_factor( 
        ( to >= 0 ) ? (1.0 / (to + 1)) : (-to+1) );
}

BEGIN_EVENT_TABLE(Window, wxFrame)
    EVT_TEXT_ENTER(Key::LowerLimitID, Window::OnLowerLimitChange)
    EVT_TEXT_ENTER(Key::UpperLimitID, Window::OnUpperLimitChange)
    EVT_CLOSE(Window::UserClosedWindow)
    EVT_TIMER(wxID_ANY, Window::timer_expired)
END_EVENT_TABLE()

std::auto_ptr<Change> Window::getState() 
{
    if ( has_3d ) std::cerr << "Warning: Only the lowest Z layer is saved when images are shown live. (Bug #170)" << std::endl;
    update_image();

    std::auto_ptr<Change> rv( new Change(keys.size()) );
    rv->do_resize = true;
    rv->resize_image.size = mkImgSize(canvas->getSize());
    for (int i = 0; i < 2; ++i)
        rv->resize_image.pixel_sizes[i] = scale_bar->get_pixel_size();

    rv->do_clear = true;
    rv->clear_image.background = background;

    rv->do_change_image = true;
    rv->image_change = *canvas->getContents();

    rv->resize_image.keys.clear();
    rv->changed_keys.clear();
    for ( Keys::const_iterator i = keys.begin(); i != keys.end(); ++i ) {
        rv->resize_image.keys.push_back( (*i)->getDeclaration() );
        rv->changed_keys.push_back( (*i)->getKeys() );
    }

    return rv;
}

void Window::OnLowerLimitChange(wxCommandEvent& e)
{
    for (Keys::iterator i = keys.begin(); i != keys.end(); ++i) {
        (*i)->OnLowerLimitChange(e);
    }
}

void Window::OnUpperLimitChange(wxCommandEvent& e)
{
    for (Keys::iterator i = keys.begin(); i != keys.end(); ++i) {
        (*i)->OnUpperLimitChange(e);
    }
}

}
}
}
