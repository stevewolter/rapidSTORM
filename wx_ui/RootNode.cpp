#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "RootNode.h"
#include "ScrolledTabNode.h"
#include "wxDisplay/wxManager.h"
#include "wxDisplay/App.h"

namespace simparm {
namespace wx_ui {

class RootFrame
: public wxFrame {
    boost::shared_ptr< Node > root_node;

public:
    RootFrame( boost::shared_ptr<Node> root_node )
        : wxFrame( NULL, wxID_ANY, wxT( PACKAGE_STRING ) ),
          root_node( root_node ) {}
};

static void create_root_frame( 
    boost::shared_ptr<Node> root_node, 
    boost::shared_ptr<RootFrame*> frame_storage,
    boost::shared_ptr<wxWindow*> root_window_view
) {
    *frame_storage = new RootFrame(root_node);
    *root_window_view = *frame_storage;
}

static void show_root_frame( 
    boost::shared_ptr<Node> root_node, 
    boost::shared_ptr<RootFrame*> frame_storage
) {
    (*frame_storage)->Show(true);
}


RootNode::RootNode( dStorm::MainThread& main_thread, const dStorm::job::Config& c ) 
: WindowNode( boost::shared_ptr<Node>() ),
  my_frame( new RootFrame*(NULL) ),
  main_thread( main_thread ),
  config(c),
  starter( &main_thread, boost::shared_ptr<simparm::Node>(), config )
{
}

RootNode::~RootNode() {
    main_thread.unregister_unstopable_job();
}

void RootNode::initialization_finished() {
    main_thread.register_unstopable_job();
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread( 
        boost::bind( &create_root_frame, shared_from_this(), my_frame, window.window ) );
    create_sizer();

    main_part.reset( new ScrolledTabNode( shared_from_this() ) );
    main_part->initialization_finished();

    starter.set_attachment_point( main_part );
    config.attach_ui( main_part );
    starter.attach_ui( config.user_interface_handle() );

    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread( 
        boost::bind( &show_root_frame, shared_from_this(), my_frame ) );
}

boost::shared_ptr<RootNode> RootNode::create( dStorm::MainThread& m, const dStorm::job::Config& c ) {
    boost::shared_ptr<RootNode> rv( new RootNode(m,c) );
    rv->initialization_finished();
    return rv;
}

}
}
