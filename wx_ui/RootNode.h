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

class MainConfig {
    dStorm::MainThread& main_thread;
    dStorm::job::Config original_config;
    boost::optional< dStorm::job::Config > config;
    boost::optional< dStorm::JobStarter > starter;
    boost::shared_ptr<Node> user_interface;

    void create_config( boost::optional<std::string> config_file );

public:
    MainConfig( dStorm::MainThread&, const dStorm::job::Config&, boost::shared_ptr<Node> );
    void serialize( std::string filename );
    void deserialize( std::string filename );
};

class RootNode 
: public Node
{
    boost::shared_ptr<RootFrame*> my_frame;
    boost::shared_ptr<Window> window_view;
    boost::shared_ptr<VisibilityControl> vc;
    dStorm::MainThread& main_thread;

    RootNode( dStorm::MainThread& );
    Relayout get_relayout_function();

    void add_attribute( simparm::BaseAttribute& ) {}
    virtual Message::Response send( Message& m ) const;
    virtual bool isActive() const { return true; }
    virtual void set_description( std::string ) {}
    virtual void set_visibility( bool ) {}
    virtual void set_user_level( UserLevel arg ) {}
    virtual void set_help_id( std::string ) {}
    virtual void set_help( std::string ) {}
    virtual void set_editability( bool ) {}
    virtual void attach_context_help( boost::shared_ptr<Window> window, std::string context_help_id );

    virtual boost::shared_ptr< Window > get_treebook_widget() { throw std::logic_error("Tree root missing"); }
    virtual boost::shared_ptr< TreeRepresentation > get_treebook_parent() { throw std::logic_error("Tree root missing"); }
    virtual void bind_visibility_group( boost::shared_ptr<Window> vg ) {}

public:
    static boost::shared_ptr<RootNode> create( dStorm::MainThread&, const dStorm::job::Config& );
    void initialization_finished();
    void add_entry_line( LineSpecification& );
    void add_full_width_line( WindowSpecification& w );
    void add_full_width_sizer( SizerSpecification& w ) { throw std::logic_error("Unexpected call"); }
    boost::shared_ptr< Window > get_parent_window() { return window_view; }
    boost::shared_ptr< VisibilityControl > get_visibility_control() { return vc; }
    NodeHandle create_object( std::string );
    NodeHandle create_group( std::string );
    ~RootNode();
};

}
}

#endif
