/* wxWidgets must be included first in Windows due to conflicts with Boost's
 * handling of windows.h. */
#include <wx/panel.h>

#include "simparm/wx_ui/WindowNode.h"
#include "simparm/wx_ui/lambda.h"
#include "simparm/wx_ui/gui_thread.h"

namespace simparm {
namespace wx_ui {

WindowNode::WindowNode( boost::shared_ptr<Node> n, std::string name ) : InnerNode(n, name) {}

boost::shared_ptr<Window> WindowNode::create_window() {
    boost::shared_ptr<Window> window( new Window() );
    run_in_GUI_thread(
        * bl::constant( window ) =
        bl::bind( bl::new_ptr<wxPanel>(), *bl::constant( InnerNode::get_parent_window() ), int(wxID_ANY) ) );
    return window;
}

void WindowNode::initialization_finished() {
    window.window = create_window();
    run_in_GUI_thread( bl::bind( &wxWindow::SetSizer, *bl::constant(window.window), 
        *bl::constant( sizer.create_sizer() ), true ) );
    InnerNode::add_full_width_line( window );
}

boost::function0<void> WindowNode::get_relayout_function() {
    return boost::function0<void>((
        bl::bind( sizer.relayout_function() ),
        bl::bind( get_parent_relayout_function() )
    ));
}

void execute_FitInside(boost::shared_ptr<bool> needs_fit_inside,
                                boost::shared_ptr<Window> window) {
    if (*needs_fit_inside) {
        static_cast<wxWindow*>(*window)->FitInside();
        *needs_fit_inside = false;
    }
}

void schedule_FitInside(boost::shared_ptr<bool> needs_fit_inside,
                                boost::shared_ptr<Window> window) {
    *needs_fit_inside = true;
    run_in_GUI_thread( bl::bind( &execute_FitInside, needs_fit_inside, window), 1 );
}

boost::function0<void> TopWindowNode::get_relayout_function() {
    return boost::function0<void>((
        bl::bind( &schedule_FitInside, needs_fit_inside, get_parent_window() )
    ));
}

}
}
