#ifndef WX_UI_WINDOW_H
#define WX_UI_WINDOW_H

#include "GUIHandle.h"
#include <boost/function/function0.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>

class wxWindow;

namespace simparm {
namespace wx_ui {

class VisibilityNode;

class Window : public boost::enable_shared_from_this<Window> {
    int frontend_non_visibles, backend_non_visibles;
    bool is_shown;
    friend class VisibilityNode;
    wxWindow* window;
    boost::function0<void> redraw;

    void update_window_show( bool called_in_gui_thread );
    void node_changed_visibility( bool became_visibility );
public:
    Window();
    ~Window();
    void change_frontend_visibility( bool );
    void set_redraw_function( boost::function0<void> r ) { redraw = r; }

    Window& operator=( wxWindow* wx_window );
    operator wxWindow*() const { return window; }
};

}
}

#endif
