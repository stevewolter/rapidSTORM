#ifndef SIMPARM_TREEENTRY_HH
#define SIMPARM_TREEENTRY_HH

#include "Object.h"
#include "Attribute.h"

namespace simparm {

class TreeObject : public Object {
  protected:
    NodeHandle create_hidden_node( simparm::NodeHandle );
  public:
    Attribute<bool> show_in_tree, force_new_root,
                    focus_immediately;

    TreeObject( std::string name, std::string desc );
};

}

#endif
