/* wxWidgets must be included first in Windows due to conflicts with Boost's
 * handling of windows.h. */
#include <wx/window.h>

#include "simparm/wx_ui/Window.h"
#include "simparm/wx_ui/gui_thread.h"
#include "simparm/wx_ui/lambda.h"

namespace simparm {
namespace wx_ui {

Window::Window() : frontend_non_visibles( 0 ), backend_non_visibles( 0 ), window(NULL) {}
Window::~Window() {
}

void Window::node_changed_visibility( bool became_visibile ) {
    backend_non_visibles += (became_visibile) ? -1 : 1;

    update_window_show( false );
}

void Window::update_window_show( bool called_in_gui_thread ) {
    assert( frontend_non_visibles >= 0 );
    assert( backend_non_visibles >= 0 );
    bool do_show = frontend_non_visibles == 0 && backend_non_visibles == 0;
    bool is_shown_is_correct = is_shown && (do_show == *is_shown);
    if ( window && ! is_shown_is_correct ) {
        if ( called_in_gui_thread ) {
            window->Show( do_show );
            if ( redraw ) { redraw(); }
        } else {
            run_in_GUI_thread( 
                bl::bind( &wxWindow::Show, bl::bind( &Window::window, * bl::constant(shared_from_this()) ), do_show ) );
            if ( redraw ) { run_in_GUI_thread( redraw); }
        }
        is_shown = do_show;
    }
}

void Window::change_frontend_visibility( bool v ) {
    frontend_non_visibles += (v) ? -1 : 1;
    update_window_show( true );
}

Window& Window::operator=( wxWindow* wx_window ) {
    window = wx_window;
    update_window_show( true );
    return *this;
}

}
}
