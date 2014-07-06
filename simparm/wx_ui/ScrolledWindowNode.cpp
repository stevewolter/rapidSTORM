/* wxWidgets must be included first in Windows due to conflicts with Boost's
 * handling of windows.h. */
#include <wx/sizer.h>

#include "simparm/wx_ui/ScrolledWindowNode.h"
#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/bind/bind.hpp>
#include "simparm/wx_ui/gui_thread.h"

namespace simparm {
namespace wx_ui {

namespace bl = boost::lambda;

bool ScrolledWindow::Destroy() {
    config.reset();
    boost::shared_ptr< dStorm::Job > my_job = job.lock();
    if ( my_job ) my_job->stop();
    job.reset();
    return true;
}

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
        bl::bind( &ScrolledWindow::mark_fit_inside, *bl::constant(scrolled_window) ),
        bl::bind( get_parent_relayout_function() )
    ));
}

void ScrolledWindowNode::set_config( boost::shared_ptr< dStorm::shell::JobFactory > m ) {
    run_in_GUI_thread(
        bl::bind( &ScrolledWindow::set_main_config, *bl::constant(scrolled_window), m ) );
}

void ScrolledWindowNode::stop_job_on_ui_detachment( boost::shared_ptr< dStorm::Job > m ) {
    run_in_GUI_thread(
        bl::bind( &ScrolledWindow::set_job, *bl::constant(scrolled_window), m ) );
}

NodeHandle ScrolledWindowNode::create_trigger( std::string name ) OVERRIDE {
    std::cerr << "Adding element " << name << std::endl;
    return WindowNode::create_trigger(name);
}

}
}
