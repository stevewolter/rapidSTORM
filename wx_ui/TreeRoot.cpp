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
    window.proportion = 1;
    run_in_GUI_thread( boost::bind( &TreeRepresentation::create_widget, tr_root, window.window, get_parent_window() ) );
    InnerNode::add_full_width_line( window );
}

}
}
