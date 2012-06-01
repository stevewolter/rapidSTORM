#include "TreeRoot.h"
#include "TreeRepresentation.h"
#include "lambda.h"

namespace simparm {
namespace wx_ui {

void TreeRoot::initialization_finished() {
    WindowSpecification window;
    treebook_widget = window.window;
    tr_root.reset( new TreeRepresentation() );
    window.name = "UNDEFINED";
    run_in_GUI_thread( boost::bind( &TreeRepresentation::create_widget, tr_root, window.window, get_parent_window() ) );
    Node::add_full_width_line( window );
}

}
}
