/* wxWidgets must be included first in Windows due to conflicts with Boost's
 * handling of windows.h. */
#include <wx/button.h>

#include "simparm/wx_ui/TriggerNode.h"
#include <boost/lexical_cast.hpp>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include "simparm/wx_ui/gui_thread.h"

namespace simparm {
namespace wx_ui {

namespace bl = boost::lambda;

class Button : public wxButton {
    boost::shared_ptr< AttributeHandle<unsigned long> > value;
public:
    Button( wxWindow* parent, std::string description, boost::shared_ptr< AttributeHandle<unsigned long> > value ) 
        : wxButton( parent, wxID_ANY, wxString( description.c_str(), wxConvUTF8 ) ), value(value) {
    }

    void button_clicked(wxCommandEvent&) { *value += 1; }

    DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(Button, wxButton)
EVT_BUTTON  (wxID_ANY, Button::button_clicked )
END_EVENT_TABLE()

void TriggerNode::initialization_finished() {
    assert( value );

    LineSpecification w( get_relayout_function() );
    run_in_GUI_thread( 
        *bl::constant( w.contents ) = 
            bl::bind( bl::new_ptr< Button >(), *bl::constant( get_parent_window() ), description, value ) );
    attach_help( w.contents );
    add_entry_line( w );
}

TriggerNode::~TriggerNode() { value->detach(); }

}
}
