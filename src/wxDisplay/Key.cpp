#include "Key.h"
#include "helpers.h"
#include <wx/dcbuffer.h>

namespace dStorm {
namespace Display {

Key::Key( wxWindow* parent, wxSize size, int num_keys )
    : wxWindow( parent, wxID_ANY, wxDefaultPosition, size ),
        current_size( size ),
        buffer( new wxBitmap(size.GetWidth(), size.GetHeight()) ),
        num_keys( num_keys ),
        max_text_width( 0 ), max_text_height( 0 ),
        colors( num_keys ),
        values( num_keys ),
        unlabelled_width(40),
        labelled_width(unlabelled_width+4),
        top_border( 10 ),
        bottom_border( 3 ),
        left_border(5),
        background_pen( this->GetBackgroundColour() ),
        background_brush( this->GetBackgroundColour() )
{
    compute_key_size();
    wxClientDC dc(this);
    text_height = dc.GetTextExtent(wxT("01234596789")).GetHeight();
}

Key::~Key() { buffer.release(); }

void Key::compute_key_size() {
    int free_height = current_size.GetHeight() 
                        - top_border - bottom_border;
    if ( free_height < 0 ) { key_distance = 0; return; }

    key_distance = 1;
    line_height = 1;
    while ( free_height < line_height*num_keys/key_distance )
        key_distance += 1;
    while ( free_height > (line_height+1)*num_keys/key_distance )
        line_height += 1;
    label_distance = int(ceil(text_height * key_distance * 1.0
                                / line_height));
}

void Key::center_rect_vertically( wxRect& centre, const wxRect& in ) 
{
    int cur_centre = (centre.GetBottom() + centre.GetTop())/2;
    int should = (in.GetBottom() + in.GetTop())/2;
    int shift = should - cur_centre;
    centre.y += shift;
}

void Key::draw_key( int index, wxDC &dc )
{
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
            float remainder; const wxChar *SI_unit_prefix;
            make_SI_prefix( values[index], remainder, SI_unit_prefix );
            wxSnprintf(cbuffer, 128, wxT("%.3g %s"), 
                remainder, SI_unit_prefix);

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

            dc.DrawText( wxString(cbuffer), text_box.GetTopLeft() );
        }
    }

void Key::draw_keys( const data_cpp::Vector<KeyChange>& kcs )
{
    wxClientDC dc( this );
    dc.SetBackground( this->GetBackgroundColour() );
    wxBufferedDC buffer( &dc, *this->buffer );
    //buffer.SetTextForeground( *wxWHITE );

    data_cpp::Vector<KeyChange>::const_iterator 
        i = kcs.begin(), end = kcs.end();

    for ( ; i != end; i++) {
        colors[i->index] = i->color;
        values[i->index] = i->value;

        draw_key( i->index, buffer );
    }
}

void Key::OnPaint( wxPaintEvent& event )
{
    wxPaintDC dc(this);

    wxRegion updateRegions = GetUpdateRegion();
    for ( wxRegionIterator region( updateRegions ); region; ++region ){
        wxRect rect = region.GetRect();

        dc.DrawBitmap(buffer->GetSubBitmap( rect ), rect.GetTopLeft());
    }
}

void Key::OnResize( wxSizeEvent& )
{
    wxSize size = GetClientSize();
    if ( size == current_size ) return;
    current_size = size;
    buffer.reset( new wxBitmap( size.GetWidth(), size.GetHeight() ) );
    wxClientDC base( this );
    wxBufferedDC dc( &base, *buffer );
    dc.SetBackground( this->GetBackgroundColour() );
    dc.Clear();

    compute_key_size();
    for (int i = 0; i < num_keys; i++)
        draw_key( i, dc );
}

void Key::resize( int new_number_of_keys ) {
    num_keys = new_number_of_keys;
    compute_key_size();

    wxClientDC base( this );
    wxBufferedDC dc( &base, *buffer );
    dc.Clear();

    for (int i = 0; i < num_keys; i++)
        draw_key( i, dc );
}

BEGIN_EVENT_TABLE(Key, wxWindow)
    EVT_PAINT(Key::OnPaint)
    EVT_SIZE(Key::OnResize)
END_EVENT_TABLE()

}
}
