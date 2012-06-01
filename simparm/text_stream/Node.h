#ifndef SIMPARM_TEXT_STREAM_NODE_H
#define SIMPARM_TEXT_STREAM_NODE_H

#include "../Node.h"
#include "../Attribute.hpp"
#include <map>
#include <memory>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include "ChildrenList.h"
#include "NodeBackend.h"

namespace simparm {
namespace text_stream {

class BackendNode;

struct Node 
: public simparm::Node, 
  public boost::enable_shared_from_this<Node>, 
  private boost::noncopyable,
  public FrontendNode
{
    Node* parent;
    std::string name, type;
    Attribute<std::string> desc;
    Attribute<bool> viewable;
    Attribute<UserLevel> userLevel;

    ChildrenList< BaseAttribute > attributes;
    boost::ptr_vector< boost::signals2::scoped_connection > connections;

    boost::shared_ptr<BackendNode> backend_node;

    std::string attribute_value_specification( const BaseAttribute& a );

    void declare_attribute( const BaseAttribute* a, std::ostream& );
    void print_attribute_value( const simparm::BaseAttribute& );
    void process_attribute( BaseAttribute&, std::istream& );
    void set_visibility( bool is ) { viewable = is; }
    void set_user_level( UserLevel arg ) { userLevel = arg; }
    void set_description( std::string d ) { desc = d; }
    virtual void set_help_id( std::string ) {}
    virtual void set_help( std::string ) {}
    virtual void set_editability( bool ) {}

    void process_attribute_command_( std::string name, std::istream& );
    void declare_( std::ostream& );

protected:
    void set_backend_node( std::auto_ptr<BackendNode> b ) { backend_node = b; }

    Node( std::string name, std::string type );
    simparm::NodeHandle adorn_node( Node* );

public:
    ~Node();

    simparm::NodeHandle create_object( std::string name );
    simparm::NodeHandle create_textfield( std::string name, std::string type );
    simparm::NodeHandle create_checkbox( std::string name );
    simparm::NodeHandle create_group( std::string name );
    simparm::NodeHandle create_tab_group( std::string name );
    simparm::NodeHandle create_choice( std::string name );
    simparm::NodeHandle create_file_entry( std::string name );
    simparm::NodeHandle create_progress_bar( std::string name );
    simparm::NodeHandle create_trigger( std::string name );
    NodeHandle create_tree_root();
    NodeHandle create_tree_object( std::string name );
    std::auto_ptr<dStorm::display::WindowHandle> get_image_window( 
        const dStorm::display::WindowProperties&, dStorm::display::DataSource& );

    void add_attribute( simparm::BaseAttribute& );
    Message::Response send( Message& m ) const;
    void initialization_finished();
    void hide();
    /** TODO: Method is deprecated and should be removed on successful migration. */
    bool isActive() const;

    NodeHandle get_handle() { return shared_from_this(); }
    boost::shared_ptr<BackendNode> get_backend() { return backend_node; }
};

}
}

#endif
