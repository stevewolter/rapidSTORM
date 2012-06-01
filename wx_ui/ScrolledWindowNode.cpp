#include "ScrolledWindowNode.h"
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/bind/bind.hpp>

namespace simparm {
namespace wx_ui {

namespace bl = boost::lambda;

static void make_scrolled_window( 
    boost::shared_ptr< wxScrolledWindow* > sw,
    boost::shared_ptr< wxWindow* > window_view,
    boost::shared_ptr< wxWindow* > parent_window
) {
    *sw = new wxScrolledWindow( *parent_window, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxHSCROLL );
    (*sw)->SetScrollRate( 5, 5 );
    *window_view = *sw;
}

void ScrolledWindowNode::initialization_finished() {
    run_in_GUI_thread(
        boost::bind( &make_scrolled_window, scrolled_window, window.window, Node::get_parent_window() ) );
    create_sizer();
    Node::add_full_width_line( window );
}

boost::function0<void> ScrolledWindowNode::get_relayout_function() {
    return boost::function0<void>(
        bl::bind( &wxScrolledWindow::FitInside, *bl::constant(scrolled_window) )
    );
}

}
}
