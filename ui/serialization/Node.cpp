#include "Node.h"
#include <boost/lexical_cast.hpp>

namespace simparm {
namespace serialization_ui {

struct TreeNode : public Node {
    std::string name;
    std::string parent_path;
    int children;

    TreeNode( boost::shared_ptr<Node> parent, std::string parent_path, std::string name ) 
        : Node(parent,parent_path + "in " + name + " "), name(name), children(0) {}
    TreeNode* get_tree_parent() { return this; }
    void initialization_finished() {
        TreeNode* p = Node::get_tree_parent();
        if ( p ) {
            parent_path = p->path;
            path = p->path + "in Output" + boost::lexical_cast<std::string>( p->children++ ) + " in " + name + " ";
        }
    }
    void serialize( std::ostream& target ) {
        if ( parent_path != "" ) {
            target << parent_path << "in ChooseTransmission in value set " << name << "\n";
        }
        Node::serialize( target );
    }
};

NodeHandle Node::create_tree_object( std::string name )
{ 
    return NodeHandle( new TreeNode( shared_from_this(), path, name ) );
}

}
}
