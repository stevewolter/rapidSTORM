#include "Window.h"
#include "gui_thread.h"
#include <wx/window.h>
#include "lambda.h"

namespace simparm {
namespace wx_ui {

Window::Window() : frontend_non_visibles( 0 ), backend_non_visibles( 0 ), is_shown(true), window(NULL) {}
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
    if ( window && do_show != is_shown ) {
        std::cerr << "Foobar" << std::endl;
        if ( called_in_gui_thread ) {
            window->Show( do_show );
            if ( redraw ) { std::cerr << "Calling redraw function" << std::endl; redraw(); }
        } else {
            run_in_GUI_thread( 
                bl::bind( &wxWindow::Show, bl::bind( &Window::window, * bl::constant(shared_from_this()) ), do_show ) );
            if ( redraw ) { std::cerr << "Calling redraw function" << std::endl; run_in_GUI_thread( redraw); }
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
