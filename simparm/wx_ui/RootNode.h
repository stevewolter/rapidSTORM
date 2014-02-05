#ifndef SIMPARM_WX_UI_ROOTNODE_H
#define SIMPARM_WX_UI_ROOTNODE_H

#include <wx/wx.h>
#include "simparm/wx_ui/Node.h"
#include <dStorm/Config.h>
#include "simparm/wx_ui/WindowNode.h"
#include "simparm/wx_ui/TabNode.h"
#include <fstream>
#include <boost/filesystem/path.hpp>
#include "shell/JobFactory.h"
#include "shell/JobMetaFactory.h"

namespace simparm {
namespace wx_ui {

class RootFrame;

class RootNode 
: public Node
{
    boost::shared_ptr<RootFrame*> my_frame;
    boost::shared_ptr<Window> window_view;
    boost::shared_ptr<VisibilityControl> vc;
    boost::optional< std::ofstream > logfile;
    ProtocolNode protocol;
    boost::optional< boost::filesystem::path > logfile_name;

    RootNode();
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
    virtual const ProtocolNode& get_protocol() const { return protocol; }

    virtual boost::shared_ptr< Window > get_treebook_widget() { throw std::logic_error("Tree root missing"); }
    virtual boost::shared_ptr< TreeRepresentation > get_treebook_parent() { throw std::logic_error("Tree root missing"); }
    virtual void bind_visibility_group( boost::shared_ptr<Window> vg ) {}

public:
    static boost::shared_ptr<RootNode> create( const dStorm::JobConfig&, boost::optional<std::string> logfile );
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
