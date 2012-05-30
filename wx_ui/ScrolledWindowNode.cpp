#include "ScrolledWindowNode.h"
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include "wxDisplay/wxManager.h"

namespace simparm {
namespace wx_ui {

static void make_scrolled_window( 
    boost::shared_ptr< wxWindow* > window_view,
    boost::shared_ptr< wxWindow* > parent_window
) {
    wxScrolledWindow *w = new wxScrolledWindow( *parent_window, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL );
    w->SetScrollRate( 0, 5 );
    *window_view = w;
}

void ScrolledWindowNode::initialization_finished() {
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
        boost::bind( &make_scrolled_window, window.window, Node::get_parent_window() ) );
    create_sizer();
    Node::add_full_width_line( window );
}

}
}
