#ifndef SIMPARM_TEXT_STREAM_NODE_H
#define SIMPARM_TEXT_STREAM_NODE_H

#include "../Node.h"
#include "../Attribute.h"
#include <map>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace simparm {
namespace text_stream {

struct Node : public simparm::Node, public boost::enable_shared_from_this<Node> {
    std::string name, type;
    Attribute<std::string> desc;
    Attribute<bool> viewable;
    Attribute<UserLevel> userLevel;

    Node* parent;
    std::vector< Node* > nodes;
    std::map< std::string, Node* > node_lookup;
    std::vector< BaseAttribute* > attributes;
    std::map< std::string, BaseAttribute* > attribute_lookup;
    bool declared;
    boost::ptr_vector< boost::signals2::scoped_connection > connections;

    void print_attribute_value( const simparm::BaseAttribute& );
    void process_attribute( BaseAttribute&, std::istream& );
    void set_visibility( bool is ) { viewable = is; }
    void set_user_level( UserLevel arg ) { userLevel = arg; }
    void set_description( std::string d ) { desc = d; }

protected:
    virtual bool print( const std::string& );
    virtual bool print_on_top_level( const std::string& );

    void add_child( Node& o );
    void remove_child( Node& o );
    void set_parent( Node& o ) { o.add_child(*this); }
    void show_attributes( std::ostream& );
    void declare(std::ostream&);
    void undeclare();

    void declare_children();

    Node( std::string name, std::string type );
    simparm::NodeHandle adorn_node( Node* );

public:
    ~Node();

    simparm::NodeHandle create_object( std::string name );
    simparm::NodeHandle create_entry( std::string name, std::string type );
    simparm::NodeHandle create_group( std::string name );
    simparm::NodeHandle create_tab_group( std::string name );
    simparm::NodeHandle create_choice( std::string name );
    simparm::NodeHandle create_file_entry( std::string name );
    simparm::NodeHandle create_progress_bar( std::string name );
    simparm::NodeHandle create_trigger( std::string name );
    void add_attribute( simparm::BaseAttribute& );
    Message::Response send( Message& m ) const;
    void initialization_finished();
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
