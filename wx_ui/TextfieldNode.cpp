#include "TextfieldNode.h"
#include <wx/textctrl.h>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

namespace simparm {
namespace wx_ui {

namespace bl = boost::lambda;

class TextCtrl : public wxTextCtrl {
    boost::shared_ptr< BaseAttributeHandle > value;
public:
    TextCtrl( wxWindow* parent, boost::shared_ptr< BaseAttributeHandle > value ) 
    : wxTextCtrl( 
        parent, wxID_ANY, 
        wxString( value->get_value().get_value_or("unset").c_str(), wxConvUTF8 ),
        wxDefaultPosition, wxDefaultSize,
        wxTE_PROCESS_ENTER
    ), value(value) {}

    void enter_pressed(wxCommandEvent&) { 
        value->set_value( std::string( GetValue().mb_str() ) );
    }

    DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(TextCtrl, wxTextCtrl)
EVT_TEXT_ENTER  (wxID_ANY, TextCtrl::enter_pressed )
END_EVENT_TABLE()

void TextfieldNode::display_value() {
    boost::optional< std::string > v = value_handle->get_value();
    if ( v )
        run_in_GUI_thread( bl::bind( &TextCtrl::ChangeValue, *bl::constant(my_window), wxString( v->c_str(), wxConvUTF8 ) ) );
}

void TextfieldNode::initialization_finished() {
    LineSpecification my_line;
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
