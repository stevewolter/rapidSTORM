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
    std::string name;

    Node* parent;
    std::vector< Node* > nodes;
    std::map< std::string, Node* > node_lookup;
    std::vector< BaseAttribute* > attributes;
    std::map< std::string, BaseAttribute* > attribute_lookup;

protected:
    void add_child( Node& o );
    void remove_child( Node& o );
    void set_parent( Node& o ) { o.add_child(*this); }

    virtual void program_options( OptionTable& );

    Node( std::string name ) : name(name), parent(NULL) {}
    simparm::NodeHandle create_node( std::string name );

public:
    ~Node();

    simparm::NodeHandle create_object( std::string name );
    simparm::NodeHandle create_entry( std::string name, std::string desc, std::string type );
    simparm::NodeHandle create_set( std::string name );
    simparm::NodeHandle create_choice( std::string name, std::string desc );
    simparm::NodeHandle create_file_entry( std::string name, std::string desc );
    simparm::NodeHandle create_progress_bar( std::string name, std::string desc );
    simparm::NodeHandle create_trigger( std::string name, std::string desc );
    virtual void add_attribute( simparm::BaseAttribute& );
    Message::Response send( Message& m ) const;
    void show() {}
    /** TODO: Method is deprecated and should be removed on successful migration. */
    bool isActive() const { return true; }

    NodeHandle get_handle() { return shared_from_this(); }

};

}
}

#endif
