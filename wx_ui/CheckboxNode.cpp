#include "CheckboxNode.h"
#include <wx/wx.h>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include "gui_thread.h"

namespace simparm {
namespace wx_ui {

namespace bl = boost::lambda;

class CheckBox : public wxCheckBox {
    boost::shared_ptr< AttributeHandle<bool> > value;
public:
    CheckBox( wxWindow* parent, boost::shared_ptr< AttributeHandle<bool> > value ) 
    : wxCheckBox( 
        parent, wxID_ANY, 
        wxT("")
    ), value(value) {
        show_program_value();
    }

    void value_changed(wxCommandEvent&) { 
        *value = GetValue();
    }

    void show_program_value() {
        boost::optional<bool> v = (*value)();
        if ( v )
            SetValue( *v );
    }

    DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(CheckBox, wxCheckBox)
EVT_CHECKBOX  (wxID_ANY, CheckBox::value_changed )
END_EVENT_TABLE()

void CheckboxNode::initialization_finished() {
    LineSpecification my_line( get_relayout_function() );

    create_static_text( my_line.label, description );
    run_in_GUI_thread( 
        *bl::constant( my_line.contents ) = *bl::constant(my_window) = 
            bl::bind( bl::new_ptr< CheckBox >(), *bl::constant( get_parent_window() ), value_handle ) );

    attach_help( my_line.label );
    attach_help( my_line.contents );
    add_entry_line( my_line );
}

CheckboxNode::~CheckboxNode() {
    value_handle->detach();
}

void CheckboxNode::add_attribute( simparm::BaseAttribute& a ) 
{
    if ( a.get_name() == "value" ) {
        value_handle.reset( new AttributeHandle<bool>(a) );
        boost::function0<void> value_change = bl::bind( &CheckBox::show_program_value, *bl::constant( my_window ) );
        connection = a.notify_on_non_GUI_value_change(       
            boost::bind( &run_in_GUI_thread, value_change ) );
    }
}

}
}
