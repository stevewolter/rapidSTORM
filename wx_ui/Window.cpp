#include "Window.h"
#include <wx/window.h>

namespace simparm {
namespace wx_ui {

Window::Window() : frontend_non_visibles( 0 ), backend_non_visibles( 0 ), window(NULL) {}
Window::~Window() {
}

void Window::node_changed_visibility( bool became_visibile ) {
    backend_non_visibles += (became_visibile) ? -1 : 1;
    update_window_show();
}

void Window::update_window_show() {
    assert( frontend_non_visibles >= 0 );
    assert( backend_non_visibles >= 0 );
    if ( window ) {
        window->Show( frontend_non_visibles == 0 && backend_non_visibles == 0 );
        if ( redraw ) redraw();
    }
}

void Window::change_frontend_visibility( bool v ) {
    frontend_non_visibles += (v) ? -1 : 1;
    update_window_show();
}

Window& Window::operator=( wxWindow* wx_window ) {
    window = wx_window;
    update_window_show();
    return *this;
}

}
}
