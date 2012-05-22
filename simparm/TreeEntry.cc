#include "Attribute.hh"
#include "TreeEntry.hh"

namespace simparm {

TreeObject::TreeObject( std::string name, std::string desc )
        : Object(name,desc),
          show_in_tree("show_in_tree", true),
          force_new_root("force_new_root", false),
          focus_immediately("focus_immediately", false) 
{
}

NodeRef TreeObject::create_hidden_node( simparm::Node& n ) {
    NodeRef r = Object::create_hidden_node( n );
    r.add_attribute( show_in_tree );
    r.add_attribute( force_new_root );
    r.add_attribute( focus_immediately );
    return r;
}

}
