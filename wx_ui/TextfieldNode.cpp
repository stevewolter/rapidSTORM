#include "TextfieldNode.h"
#include <wx/wx.h>
#include "wxDisplay/wxManager.h"

namespace simparm {
namespace wx_ui {

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

void make_label( boost::shared_ptr<wxStaticText*> label, boost::shared_ptr< wxWindow* > parent_window, std::string text ) {
    *label = new wxStaticText( *parent_window, wxID_ANY, wxString( text.c_str(), wxConvUTF8 ) );
}
void make_text_object( 
    boost::shared_ptr<wxWindow*> text_object, 
    boost::shared_ptr< wxWindow* > parent_window, 
    boost::shared_ptr< BaseAttributeHandle > value
) {
    *text_object = new TextCtrl( *parent_window, value );
}

void make_unit_object( 
    boost::shared_ptr<wxWindow*> text_object, 
    boost::shared_ptr< wxWindow* > parent_window, 
    std::string unit
) {
    (*text_object) = new wxStaticText( *parent_window, wxID_ANY, wxString( unit.c_str(), wxConvUTF8 ) );
}

void TextfieldNode::initialization_finished() {
    LineSpecification my_line;
    my_window = my_line.contents;
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
        boost::bind( &make_label, my_line.label, get_parent_window(), description ) );
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
        boost::bind( &make_text_object, my_line.contents, get_parent_window(), value_handle ) );
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
        boost::bind( &make_unit_object, my_line.adornment, get_parent_window(), unit ) );
    add_entry_line( my_line );
}

TextfieldNode::~TextfieldNode() {
    value_handle->detach();
}

}
}
