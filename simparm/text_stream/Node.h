#ifndef SIMPARM_TEXT_STREAM_NODE_H
#define SIMPARM_TEXT_STREAM_NODE_H

#include "../Node.h"
#include "../BaseAttribute.h"
#include <map>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace simparm {
namespace text_stream {

struct Node : public simparm::Node, public boost::enable_shared_from_this<Node> {
    std::string name, type;

    Node* parent;
    std::vector< Node* > nodes;
    std::map< std::string, Node* > node_lookup;
    std::vector< BaseAttribute* > attributes;
    std::map< std::string, BaseAttribute* > attribute_lookup;
    bool declared;
    boost::ptr_vector< boost::signals2::scoped_connection > connections;

    void print_attribute_value( const simparm::BaseAttribute& );
    void process_attribute( BaseAttribute&, std::istream& );

protected:
    virtual bool print( const std::string& );
    virtual bool print_on_top_level( const std::string& );

    void add_child( Node& o );
    void remove_child( Node& o );
    void set_parent( Node& o ) { o.add_child(*this); }
    void show_children();
    void show_attributes( std::ostream& );
    void declare(std::ostream&);
    void undeclare();

    Node( std::string name, std::string type ) : name(name), type(type), parent(NULL), declared(false) {}
    simparm::NodeHandle create_node( std::string name, std::string type );

public:
    ~Node();

    simparm::NodeHandle create_object( std::string name );
    simparm::NodeHandle create_entry( std::string name, std::string desc, std::string type );
    simparm::NodeHandle create_set( std::string name );
    simparm::NodeHandle create_choice( std::string name, std::string desc );
    simparm::NodeHandle create_file_entry( std::string name, std::string desc );
    simparm::NodeHandle create_progress_bar( std::string name, std::string desc );
    simparm::NodeHandle create_trigger( std::string name, std::string desc );
    void add_attribute( simparm::BaseAttribute& );
    Message::Response send( Message& m ) const;
    void show();
    void hide();
    /** TODO: Method is deprecated and should be removed on successful migration. */
    bool isActive() const;

    void processCommand( std::istream& );
    virtual void processCommand( const std::string&, std::istream& );
    NodeHandle get_handle() { return shared_from_this(); }
};

}
}

#endif
