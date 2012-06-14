#include "TreeRepresentation.h"
#include "TreeRoot.h"
#include "lambda.h"
#include "gui_thread.h"

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

boost::function0<void> TreeRoot::get_relayout_function() {
    return boost::function0<void>( ( 
        boost::bind( &TreeRepresentation::InvalidateBestSize, tr_root ),
        bl::bind( InnerNode::get_relayout_function() )
    ) );
}

}
}
