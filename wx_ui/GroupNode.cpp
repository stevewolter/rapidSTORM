#include "GroupNode.h"
#include <wx/statbox.h>
#include <wx/sizer.h>
#include <wx/gbsizer.h>
#include "lambda.h"

namespace simparm {
namespace wx_ui {

static void create_group_sizer(
    boost::shared_ptr< wxSizer* > box_sizer,
    boost::shared_ptr< wxWindow* > parent_window,
    boost::shared_ptr< wxGridBagSizer* > children_sizer,
    wxString description
) {
    *box_sizer = new wxStaticBoxSizer( wxVERTICAL, *parent_window, description );
    (*box_sizer)->Add( *children_sizer, 1, wxEXPAND );
}

void GroupNode::initialization_finished() {
    WindowNode::create_unattached_gridbag_sizer();
    run_in_GUI_thread( boost::bind( create_group_sizer, 
        box_sizer, Node::get_parent_window(), WindowNode::sizer, wxString( description.c_str(), wxConvUTF8 ) ) );
    SizerSpecification s;
    s.sizer = box_sizer;
    Node::add_full_width_sizer( s );
}

}
}
