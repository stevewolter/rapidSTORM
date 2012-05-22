#ifndef SIMPARM_TREEENTRY_HH
#define SIMPARM_TREEENTRY_HH

#include "Object.hh"
#include "Attribute.hh"

namespace simparm {

class TreeObject : public Object {
  protected:
    NodeRef create_hidden_node( simparm::Node& );
  public:
    Attribute<bool> show_in_tree, force_new_root,
                    focus_immediately;

    TreeObject( std::string name, std::string desc );
};

}

#endif
