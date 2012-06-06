#ifndef SIMPARM_SERIALIZATION_UI_NODE_H
#define SIMPARM_SERIALIZATION_UI_NODE_H

#include <simparm/Node.h>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/range/algorithm/for_each.hpp>

namespace simparm {
namespace serialization_ui {

class TreeNode;

class Node : public simparm::Node, public boost::enable_shared_from_this<Node> {
    boost::shared_ptr<Node> parent;
    simparm::BaseAttribute* value;
    std::vector< Node* > children;

    NodeHandle make_node( std::string name ) {
        return NodeHandle( new Node( shared_from_this(), path + "in " + name + " " ) );
    }
protected:
    std::string path;
    Node( boost::shared_ptr<Node> parent, std::string path )
        : parent(parent), value(NULL), path(path) { if (parent) parent->children.push_back(this); }
public:
    static boost::shared_ptr<Node> create_root_node() {
        return boost::shared_ptr<Node>( new Node( boost::shared_ptr<Node>(), "" ) );
    }
    virtual ~Node() {}
    virtual NodeHandle create_object( std::string name ) { return make_node( name ); }
    virtual NodeHandle create_textfield( std::string name, std::string ) { return make_node(name); }
    virtual NodeHandle create_checkbox( std::string name ) { return make_node( name); }
    virtual NodeHandle create_group( std::string name ) { return make_node( name ); }
    virtual NodeHandle create_tab_group( std::string name ) { return make_node( name ); }
    virtual NodeHandle create_choice( std::string name ) { return make_node(name); }
    virtual NodeHandle create_file_entry( std::string name ) { return make_node(name); }
    virtual NodeHandle create_progress_bar( std::string name ) { return make_node( name ); }
    virtual NodeHandle create_trigger( std::string name ) { return make_node( name ); }
    virtual NodeHandle create_tree_root() { return shared_from_this(); }
    virtual NodeHandle create_tree_object( std::string name );
    virtual std::auto_ptr<dStorm::display::WindowHandle> get_image_window( 
        const dStorm::display::WindowProperties&, dStorm::display::DataSource& )
    {
        throw std::runtime_error("Serialization user interface was asked to produce a display window");
    }

    virtual void add_attribute( simparm::BaseAttribute& a ) {
        if ( a.get_name() == "value" ) {
            value = &a;
        }
    }
    virtual Message::Response send( Message& m ) const { return Message::OKYes; }
    virtual void initialization_finished() {}
    /** TODO: Method is deprecated and should be removed on successful migration. */
    virtual bool isActive() const { return false; }
    virtual void set_description( std::string ) {}
    virtual void set_visibility( bool ) {}
    virtual void set_user_level( UserLevel arg ) {}
    virtual void set_help_id( std::string ) {}
    virtual void set_help( std::string ) {}
    virtual void set_editability( bool ) {}

    virtual TreeNode* get_tree_parent() { return (parent) ? parent->get_tree_parent() : NULL; }
    virtual void serialize( std::ostream& target ) {
        if ( value ) {
            target << path << "in value ";
            boost::optional<std::string> v = value->get_value();
            if ( v )
                target << "set " << *v;
            else
                target << "unset";
            target << "\n";
        }
        boost::range::for_each( children, boost::bind( &Node::serialize, _1, boost::ref(target) ) );
    }
};

}
}

#endif
