#include "simparm/wx_ui/TreeRepresentation.h"
#include "simparm/wx_ui/TreeRoot.h"
#include "simparm/wx_ui/lambda.h"
#include "simparm/wx_ui/gui_thread.h"

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
        bl::bind( &TreeRepresentation::InvalidateBestSize, *bl::constant(tr_root) ),
        bl::bind( get_parent_relayout_function() )
    ) );
}

}
}
