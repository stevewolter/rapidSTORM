#ifndef SIMPARM_CMDLINE_UI_NODE_H
#define SIMPARM_CMDLINE_UI_NODE_H

#include "../Node.h"
#include "../BaseAttribute.h"
#include <map>
#include <boost/enable_shared_from_this.hpp>

namespace simparm {
namespace cmdline_ui {

class OptionTable;

struct Node : public simparm::Node, public boost::enable_shared_from_this<Node> {
    std::string name, description;
    bool visible;

    Node* parent;
    std::vector< Node* > nodes;

protected:
    virtual void add_child( Node& o );
    virtual void remove_child( Node& o );
    void set_parent( Node& o ) { o.add_child(*this); }

    Node( std::string name ) : name(name), visible(true), parent(NULL) {}
    simparm::NodeHandle adorn_node( Node* );

    std::string get_description() const { return description; }

    bool is_visible() { return visible; }

public:
    ~Node();

    virtual void program_options( OptionTable& );

    simparm::NodeHandle create_object( std::string name );
    simparm::NodeHandle create_textfield( std::string name, std::string type );
    simparm::NodeHandle create_checkbox( std::string name );
    simparm::NodeHandle create_group( std::string name );
    simparm::NodeHandle create_tab_group( std::string name );
    simparm::NodeHandle create_choice( std::string name );
    simparm::NodeHandle create_file_entry( std::string name );
    simparm::NodeHandle create_progress_bar( std::string name );
    simparm::NodeHandle create_trigger( std::string name );
    virtual void add_attribute( simparm::BaseAttribute& );
    Message::Response send( Message& m ) const;
    void initialization_finished() {}
    /** TODO: Method is deprecated and should be removed on successful migration. */
    bool isActive() const { return true; }
    void set_description( std::string d ) { description = d; }
    void set_visibility( bool v ) { visible = v; }
    void set_user_level( UserLevel ) {}
    void set_help_id( std::string ) {}
    void set_help( std::string ) {}
    void set_editability( bool ) {}

    NodeHandle get_handle() { return shared_from_this(); }

};

}
}

#endif
