#include "TreePage.h"
#include "TreeRepresentation.h"

namespace simparm {
namespace wx_ui {

void TreePage::add_entry_line( const LineSpecification& s ) { throw std::logic_error("Expected full window widget in tree page"); }

void TreePage::add_full_width_line( WindowSpecification w )  {
    tr_node.reset( new TreeRepresentation() );
    run_in_GUI_thread( boost::bind( &TreeRepresentation::add_as_child, tr_node, Node::get_treebook_parent(), w, get_relayout_function() ) );
}

void TreePage::add_full_width_sizer( SizerSpecification ) { throw std::logic_error("Expected full window widget in tree page"); }

}
}
