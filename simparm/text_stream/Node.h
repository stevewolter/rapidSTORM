#ifndef SIMPARM_TEXT_STREAM_NODE_H
#define SIMPARM_TEXT_STREAM_NODE_H

#include "../Node.hh"
#include "../BaseAttribute.hh"
#include <map>
#include <boost/ptr_container/ptr_vector.hpp>

namespace simparm {
namespace text_stream {

struct Node : public simparm::Node {
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
    std::auto_ptr<simparm::Node> create_node( std::string name, std::string type );

public:
    ~Node();

    std::auto_ptr<simparm::Node> create_object( std::string name );
    std::auto_ptr<simparm::Node> create_entry( std::string name, std::string desc, std::string type );
    std::auto_ptr<simparm::Node> create_set( std::string name );
    std::auto_ptr<simparm::Node> create_choice( std::string name, std::string desc );
    std::auto_ptr<simparm::Node> create_file_entry( std::string name, std::string desc );
    std::auto_ptr<simparm::Node> create_progress_bar( std::string name, std::string desc );
    std::auto_ptr<simparm::Node> create_trigger( std::string name, std::string desc );
    void add_attribute( simparm::BaseAttribute& );
    Message::Response send( Message& m );
    void show();
    void hide();
    /** TODO: Method is deprecated and should be removed on successful migration. */
    bool isActive() const;

    void processCommand( std::istream& );
    virtual void processCommand( const std::string&, std::istream& );
};

}
}

#endif
