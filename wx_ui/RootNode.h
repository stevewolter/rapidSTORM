#ifndef SIMPARM_WX_UI_ROOTNODE_H
#define SIMPARM_WX_UI_ROOTNODE_H

#include <wx/wx.h>
#include "Node.h"
#include "MainThread.h"
#include "job/Config.h"
#include "WindowNode.h"
#include "TabNode.h"
#include "JobStarter.h"

namespace simparm {
namespace wx_ui {

class RootFrame;

class RootNode 
: public Node
{
    boost::shared_ptr<RootFrame*> my_frame;
    boost::shared_ptr<wxWindow*> window_view;
    dStorm::MainThread& main_thread;
    dStorm::job::Config config;
    dStorm::JobStarter starter;

    boost::shared_ptr< Node > main_part;

    RootNode( dStorm::MainThread&, const dStorm::job::Config& );

public:
    static boost::shared_ptr<RootNode> create( dStorm::MainThread&, const dStorm::job::Config& );
    void initialization_finished();
    void add_entry_line( const LineSpecification& );
    void add_full_width_line( WindowSpecification w );
    boost::shared_ptr< wxWindow* > get_parent_window() { return window_view; }
    NodeHandle create_object( std::string );
    NodeHandle create_group( std::string );
    ~RootNode();
};

}
}

#endif
