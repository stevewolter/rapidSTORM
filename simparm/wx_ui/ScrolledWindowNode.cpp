#include "ScrolledWindowNode.h"
#include <wx/sizer.h>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/bind/bind.hpp>
#include "gui_thread.h"

namespace simparm {
namespace wx_ui {

namespace bl = boost::lambda;

void ScrolledWindow::mark_fit_inside() { 
    needs_fit_inside = true; 
    run_in_GUI_thread( bl::bind( &ScrolledWindow::do_fit_inside, this ), 1 );
}

void ScrolledWindow::do_fit_inside() { 
    if ( needs_fit_inside ) { FitInside(); needs_fit_inside = false; }
}

static void make_scrolled_window( 
    boost::shared_ptr< ScrolledWindow* > sw,
    boost::shared_ptr< Window > window_view,
    boost::shared_ptr< Window > parent_window
) {
    *sw = new ScrolledWindow( *parent_window );
    (*sw)->SetScrollRate( 5, 5 );
    *window_view = *sw;
}

boost::shared_ptr<Window> ScrolledWindowNode::create_window() {
    boost::shared_ptr<Window> window( new Window() );
    run_in_GUI_thread(
        boost::bind( &make_scrolled_window, scrolled_window, window, InnerNode::get_parent_window() ) );
    return window;
}

boost::function0<void> ScrolledWindowNode::get_relayout_function() {
    return boost::function0<void>((
        bl::bind( &ScrolledWindow::mark_fit_inside, *bl::constant(scrolled_window) )
    ));
}

void ScrolledWindowNode::set_config( std::auto_ptr< MainConfig > m ) {
    run_in_GUI_thread(
        bl::bind( &ScrolledWindow::set_main_config, *bl::constant(scrolled_window), m.release() ) );
}

}
}
