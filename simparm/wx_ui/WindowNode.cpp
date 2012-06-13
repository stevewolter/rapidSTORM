#include "WindowNode.h"
#include <wx/wx.h>
#include "lambda.h"
#include "gui_thread.h"

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

Relayout WindowNode::get_relayout_function() {
    return boost::function0<void>( ( 
        bl::bind( sizer.get_relayout_function() ),
        std::cerr << bl::constant("Resizing window ") << *bl::constant(box_sizer) << "\n",
        bl::bind( InnerNode::get_relayout_function() )
    ) );
}

}
}
