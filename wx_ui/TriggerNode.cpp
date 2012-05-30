#include "TriggerNode.h"
#include <wx/wx.h>
#include "wxDisplay/wxManager.h"
#include <boost/lexical_cast.hpp>
#include "detach_window.h"

namespace simparm {
namespace wx_ui {

class Button : public wxButton {
    simparm::Attribute<unsigned long>& value;
public:
    Button( wxWindow* parent, std::string description, simparm::Attribute<unsigned long>& value ) 
        : wxButton( parent, wxID_ANY, wxString( description.c_str(), wxConvUTF8 ) ), value(value) {
    }

    void button_clicked(wxCommandEvent&) { value = value() + 1; }

    DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(Button, wxButton)
EVT_BUTTON  (wxID_ANY, Button::button_clicked )
END_EVENT_TABLE()

static void make_button( 
    boost::shared_ptr<wxWindow*> target, 
    boost::shared_ptr< wxWindow* > parent_window, 
    std::string description,
    simparm::Attribute<unsigned long>* value
) {
    *target = new Button( *parent_window, description, *value );
}

void TriggerNode::initialization_finished() {
    assert( value );

    LineSpecification w;
    my_window = w.contents;
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
        boost::bind( &make_button, w.contents, get_parent_window(), description, value ) );
    add_entry_line( w );
}

TriggerNode::~TriggerNode() { wait_for_window_detachment( my_window ); }

}
}
