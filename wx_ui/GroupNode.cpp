#include "GroupNode.h"
#include <wx/statbox.h>
#include <wx/sizer.h>
#include <wx/gbsizer.h>
#include "lambda.h"
#include "gui_thread.h"

namespace simparm {
namespace wx_ui {

static void create_group_sizer(
    boost::shared_ptr< wxSizer* > box_sizer,
    boost::shared_ptr< Window > box,
    boost::shared_ptr< Window > parent_window,
    boost::shared_ptr< wxSizer* > children_sizer,
    wxString description
) {
    wxStaticBox* sb =new wxStaticBox( *parent_window, wxID_ANY, description );
    *box_sizer = new wxStaticBoxSizer( sb, wxVERTICAL );
    (*box_sizer)->Add( *children_sizer, 1, wxEXPAND );
    *box = sb;
}

void GroupNode::initialization_finished() {
    boost::shared_ptr< Window > box( new Window() );
    GUIHandle<wxSizer> children_sizer = sizer.create_sizer();
    run_in_GUI_thread( boost::bind( create_group_sizer, 
        box_sizer, box, InnerNode::get_parent_window(), children_sizer, wxString( description.c_str(), wxConvUTF8 ) ) );
    SizerSpecification s;
    s.sizer = box_sizer;
    // TODO: Dirty hack needs to be replaced by connection to backend
    if ( description == "Output options" ) s.proportion = 1;
    bind_visibility_group( box );
    InnerNode::add_full_width_sizer( s );
}

void GroupNode::add_entry_line( LineSpecification& l ) {
    bind_visibility_group( l.label );
    bind_visibility_group( l.contents );
    bind_visibility_group( l.adornment );
    sizer.add_entry_line( l );
}
void GroupNode::add_full_width_line( WindowSpecification& w ) {
    bind_visibility_group( w.window );
    sizer.add_full_width_line( w );
}

}
}
