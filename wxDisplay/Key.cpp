#include "Key.h"
#include "helpers.h"
#include <wx/dcbuffer.h>
#include <algorithm>
#include <math.h>

#include "debug.h"

namespace dStorm {
namespace Display {

Key::Key( int number, wxWindow* parent, wxSize size, 
          const Declaration& decl )
    : wxWindow( parent, wxID_ANY, wxDefaultPosition, size ),
        parent(parent),
        lowerBoundary(NULL), upperBoundary(NULL),
        current_size( size ),
        buffer( new wxBitmap(size.GetWidth(), size.GetHeight()) ),
        num_keys( decl.size ),
        max_text_width( 0 ), max_text_height( 0 ),
        colors( num_keys ),
        values( num_keys ),
        unlabelled_width(40),
        labelled_width(unlabelled_width+4),
        top_border( 10 ),
        bottom_border( 3 ),
        left_border(5),
        background_pen( this->GetBackgroundColour() ),
        background_brush( this->GetBackgroundColour() ),
        current_declaration( decl ),
        source(NULL),
        key_index( number )
{
    compute_key_size();
    wxClientDC dc(this);
    text_height = dc.GetTextExtent(wxT("01234596789")).GetHeight();

    wxString str( decl.unit.c_str(), wxConvUTF8 );
    label = new wxStaticText( parent, wxID_ANY, wxT("Key (in ") + str + wxT(")"));
    cursor = new wxStaticText( parent, wxID_ANY, wxT("        ") );
    if ( decl.can_set_lower_limit ) 
        lowerBoundary = new wxTextCtrl( parent, LowerLimitID, 
            wxString( decl.lower_limit.c_str(), wxConvUTF8 ), 
            wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
    if ( decl.can_set_upper_limit ) 
        upperBoundary = new wxTextCtrl( parent, UpperLimitID, 
            wxString( decl.upper_limit.c_str(), wxConvUTF8 ), 
            wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
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
        int reverse_index = num_keys - i->index - 1;
        colors[reverse_index] = i->color;
        values[reverse_index] = i->value;

        draw_key( reverse_index, buffer );
    }
}

void Key::OnPaint( wxPaintEvent& event )
{
    DEBUG("Begin OnPaint");
    wxPaintDC dc(this);

    wxRegion updateRegions = GetUpdateRegion();
    for ( wxRegionIterator region( updateRegions ); region; ++region ){
        wxRect rect = region.GetRect();

        dc.DrawBitmap(buffer->GetSubBitmap( rect ), rect.GetTopLeft());
    }
    DEBUG("End OnPaint");
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

void Key::resize( const Declaration &decl ) {
    num_keys = decl.size;
    current_declaration = decl;
    compute_key_size();

    wxClientDC base( this );
    wxBufferedDC dc( &base, *buffer );
    dc.Clear();

    for (int i = 0; i < num_keys; i++)
        draw_key( i, dc );

    wxString str( decl.unit.c_str(), wxConvUTF8 );
    label->SetLabel(wxT("Key (in ") + str + wxT(")"));
}

BEGIN_EVENT_TABLE(Key, wxWindow)
    EVT_PAINT(Key::OnPaint)
    EVT_SIZE(Key::OnResize)
END_EVENT_TABLE()

dStorm::Display::KeyDeclaration Key::getDeclaration() const
{
    return current_declaration;
}

data_cpp::Vector<KeyChange>
    Key::getKeys() const
{
    data_cpp::Vector<KeyChange> rv;
    KeyChange *ne = rv.allocate(num_keys);
    for (int i = 0; i < num_keys; i++) {
        int rev = num_keys - i - 1;
        ne[rev].index = rev;
        ne[rev].color = colors[i];
        ne[rev].value = values[i];
    }
    rv.commit(num_keys);
    return rv;
}

struct DistanceTo {
    const Color compare_to;

    DistanceTo(Color c) : compare_to(c) {}
    bool operator()(const Color& a, const Color& b) {
        return (a.distance(compare_to) < b.distance(compare_to));
    }
};

void Key::cursor_value( const DataSource::PixelInfo& info, float value )
{
    bool approx = false;
    if ( std::isnan(value) ) {
        /* Search for color in key closest to info.color */
        std::vector<Color>::iterator element = 
            std::min_element( colors.begin(), colors.end(), 
                              DistanceTo(info.pixel) );
        
        value = values[ element - colors.begin() ];
        approx = true;
    }

    wxChar cbuffer[128];
    float remainder; const wxChar *SI_unit_prefix;
    make_SI_prefix( value, remainder, SI_unit_prefix );
    wxSnprintf(cbuffer, 128, wxT("%.3g %s"), 
        remainder, SI_unit_prefix);
    cursor->SetLabel( ((approx) ? wxT("ca. ") : wxT("")) + wxString(cbuffer) );
}

wxBoxSizer *Key::getBox()
{
        wxBoxSizer *key = new wxBoxSizer( wxVERTICAL );
        key->Add( getLabel(), 
                  wxSizerFlags().Center().Border( wxALL, 10 ) );
        key->Add( this, wxSizerFlags(1).Expand() );
        key->Add( getCursorText(), 
                  wxSizerFlags().Center().Border( wxALL, 10 ) );
        if ( lowerBoundary || upperBoundary )
            key->Add( new wxStaticText( parent, wxID_ANY, wxT("Color range") ) );

        if ( lowerBoundary )
            key->Add( lowerBoundary, wxSizerFlags().Center().Border( wxALL, 10 ) );
        if ( upperBoundary )
            key->Add( upperBoundary, wxSizerFlags().Center().Border( wxALL, 10 ) );
    return key;
}

void Key::OnLowerLimitChange( wxCommandEvent& ) {
    if ( source && lowerBoundary ) {
        source->notice_user_key_limits( key_index, true,
            std::string(lowerBoundary->GetValue().mb_str()) );
    }
}
void Key::OnUpperLimitChange( wxCommandEvent& ) {
    if ( source && upperBoundary )
        source->notice_user_key_limits( key_index, false,
            std::string(upperBoundary->GetValue().mb_str()) );
}

}
}
