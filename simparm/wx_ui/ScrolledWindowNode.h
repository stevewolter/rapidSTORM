#ifndef SIMPARM_WX_UI_SCROLLEDWINDOWNODE_H
#define SIMPARM_WX_UI_SCROLLEDWINDOWNODE_H

#include "WindowNode.h"
#include "GUIHandle.h"
#include "MainConfig.h"
#include <wx/scrolwin.h>

namespace simparm {
namespace wx_ui {

class ScrolledWindow : public wxScrolledWindow {
    bool needs_fit_inside;
    std::auto_ptr< MainConfig > config;
public:
    ScrolledWindow( wxWindow* parent )
        : wxScrolledWindow( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxHSCROLL ),
          needs_fit_inside(false) {}
    bool Destroy() { config.reset(); return true; }
    void mark_fit_inside();
    void do_fit_inside();
    void set_main_config( MainConfig* m ) { config.reset(m); }
    void serialize( std::string filename ) { config->serialize( filename ); }
};

class ScrolledWindowNode : public WindowNode {
    GUIHandle< ScrolledWindow > scrolled_window;
    virtual boost::shared_ptr<Window> create_window();
public:
    ScrolledWindowNode( boost::shared_ptr<Node> n, std::string name ) 
        : WindowNode(n, name) {}
    boost::function0<void> get_relayout_function() ;
    void set_config( std::auto_ptr< MainConfig > );
};

}
}

#endif
