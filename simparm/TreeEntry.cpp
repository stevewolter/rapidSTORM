#include "Attribute.h"
#include "TreeEntry.h"
#include "Node.h"

namespace simparm {

TreeObject::TreeObject( std::string name, std::string desc )
        : Object(name,desc),
          show_in_tree("show_in_tree", true),
          force_new_root("force_new_root", false),
          focus_immediately("focus_immediately", false) 
{
}

NodeHandle TreeObject::create_hidden_node( simparm::NodeHandle n ) {
    NodeHandle r = Object::create_hidden_node( n );
    r->add_attribute( show_in_tree );
    r->add_attribute( force_new_root );
    r->add_attribute( focus_immediately );
    return r;
}

NodeHandle TreeObject::make_naked_node( simparm::NodeHandle n ) {
    return NodeHandle( n->create_tree_object( getName() ) );
}

}
