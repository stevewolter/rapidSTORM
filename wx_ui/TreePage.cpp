#include "TreePage.h"
#include "TreeRepresentation.h"

namespace simparm {
namespace wx_ui {

void TreePage::add_entry_line( LineSpecification& s ) { throw std::logic_error("Expected full window widget in tree page"); }

void TreePage::add_full_width_line( WindowSpecification& w )  {
    tr_node.reset( new TreeRepresentation() );
    wxString name( w.name.c_str(), wxConvUTF8 );
    run_in_GUI_thread( boost::bind( &TreeRepresentation::add_as_child, tr_node,
            InnerNode::get_treebook_parent(), w.window, name, get_relayout_function() ) );
}

void TreePage::add_full_width_sizer( SizerSpecification& ) { throw std::logic_error("Expected full window widget in tree page"); }

}
}
