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
: public WindowNode
{
    boost::shared_ptr<RootFrame*> my_frame;
    dStorm::MainThread& main_thread;
    dStorm::job::Config config;
    dStorm::JobStarter starter;

    boost::shared_ptr< Node > main_part;

    RootNode( dStorm::MainThread&, const dStorm::job::Config& );

public:
    static boost::shared_ptr<RootNode> create( dStorm::MainThread&, const dStorm::job::Config& );
    void initialization_finished();
    ~RootNode();
};

}
}

#endif
