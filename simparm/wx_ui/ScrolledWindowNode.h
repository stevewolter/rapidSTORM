#ifndef SIMPARM_WX_UI_SCROLLEDWINDOWNODE_H
#define SIMPARM_WX_UI_SCROLLEDWINDOWNODE_H

#include <wx/scrolwin.h>
#include "simparm/wx_ui/WindowNode.h"
#include "simparm/wx_ui/GUIHandle.h"
#include "shell/JobFactory.h"

namespace simparm {
namespace wx_ui {

class ScrolledWindow : public wxScrolledWindow {
    bool needs_fit_inside;
    boost::shared_ptr< dStorm::shell::JobFactory > config;
    boost::weak_ptr< dStorm::Job > job;
public:
    ScrolledWindow( wxWindow* parent )
        : wxScrolledWindow( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxHSCROLL ),
          needs_fit_inside(false) {}
    bool Destroy();
    void mark_fit_inside();
    void do_fit_inside();
    void set_main_config( boost::shared_ptr< dStorm::shell::JobFactory > m ) { config = m; job.reset(); }
    void set_job( boost::shared_ptr< dStorm::Job > m ) { config.reset(); job = m; }
    void serialize( std::string filename ) { if ( config ) config->serialize( filename ); }
};

class ScrolledWindowNode : public WindowNode {
    boost::shared_ptr<Node> outer_window;
    GUIHandle< ScrolledWindow > scrolled_window;
    virtual boost::shared_ptr<Window> create_window();
public:
    ScrolledWindowNode( boost::shared_ptr<Node> n, std::string name ) 
        : WindowNode(n, name), outer_window(n) {
        set_self_growing();
    }
    void set_description( std::string d ) OVERRIDE {
        outer_window->set_description(d);
        WindowNode::set_description(d);
    }
    void initialization_finished() OVERRIDE {
        outer_window->initialization_finished();
        WindowNode::initialization_finished();
    }
    NodeHandle create_trigger( std::string name ) {
        if (name == "Run") {
            return outer_window->create_trigger(name);
        } else {
            return WindowNode::create_trigger(name);
        }
    }
    boost::function0<void> get_relayout_function() ;
    void set_config( boost::shared_ptr< dStorm::shell::JobFactory > );
    void stop_job_on_ui_detachment( boost::shared_ptr<dStorm::Job> );
};

}
}

#endif
