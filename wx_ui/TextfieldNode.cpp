#include "TextfieldNode.h"
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

namespace simparm {
namespace wx_ui {

namespace bl = boost::lambda;

class TextCtrl : public wxPanel {
    wxTextCtrl* text;
    wxCheckBox* optional;
    boost::shared_ptr< BaseAttributeHandle > value;
    wxColour normal_bg, uncommitted_bg;

public:
    TextCtrl( wxWindow* parent, boost::shared_ptr< BaseAttributeHandle > value ) 
    : wxPanel( parent, wxID_ANY ),
      text( new wxTextCtrl(
        this, wxID_ANY, 
        wxT(""),
        wxDefaultPosition, wxDefaultSize,
        wxTE_PROCESS_ENTER)),
      optional( new wxCheckBox( this, wxID_ANY, wxT("") ) ),
      value(value),
      uncommitted_bg( 255, 200, 200 )
    {
        normal_bg = text->GetBackgroundColour();
        optional->Show( value->value_is_optional() );
        boost::optional< std::string > v = value->get_value();
        if ( ! v ) {
            text->Hide();
        } else {
            text->ChangeValue( wxString( v->c_str(), wxConvUTF8 ) );
        }

        wxBoxSizer* outer_sizer = new wxBoxSizer( wxHORIZONTAL );
        outer_sizer->Add( optional );
        outer_sizer->Add( text, 1, wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN );
        SetSizer( outer_sizer );
    }

    void enter_pressed(wxCommandEvent&) { 
        bool success = value->set_value( std::string( text->GetValue().mb_str() ) );
        text->SetBackgroundColour( (success) ? normal_bg : uncommitted_bg );
        text->ClearBackground();
    }

    void checkbox_marked( wxCommandEvent& ) {
        if ( optional->GetValue() ) {
            text->Show();
            text->SetBackgroundColour( uncommitted_bg );
            text->ClearBackground();
        } else {
            text->Hide();
            value->unset_value();
        }
    }

    void ChangeValue( const wxString& s ) { text->ChangeValue(s); }

    DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(TextCtrl, wxPanel)
EVT_TEXT_ENTER  (wxID_ANY, TextCtrl::enter_pressed )
EVT_CHECKBOX    (wxID_ANY, TextCtrl::checkbox_marked )
END_EVENT_TABLE()

void TextfieldNode::display_value() {
    boost::optional< std::string > v = value_handle->get_value();
    if ( v )
        run_in_GUI_thread( bl::bind( &TextCtrl::ChangeValue, *bl::constant(my_window), wxString( v->c_str(), wxConvUTF8 ) ) );
}

void TextfieldNode::initialization_finished() {
    LineSpecification my_line( get_relayout_function() );
    create_static_text( my_line.label, description );
    run_in_GUI_thread( 
        *bl::constant( my_line.contents ) = 
        *bl::constant( my_window ) = 
            bl::bind( bl::new_ptr< TextCtrl >(), *bl::constant(get_parent_window()), value_handle ) );
    create_static_text( my_line.adornment, unit );
    add_entry_line( my_line );
}

TextfieldNode::~TextfieldNode() {
    value_handle->detach();
}

void TextfieldNode::add_attribute( simparm::BaseAttribute& a ) {
    if ( a.get_name() == "value" ) {
        value_handle.reset( new BaseAttributeHandle(a) );
        connection = a.notify_on_non_GUI_value_change( boost::bind( &TextfieldNode::display_value, this ) );
    }
    if ( a.get_name() == "unit_symbol" ) unit = *a.get_value();
}

}
}
