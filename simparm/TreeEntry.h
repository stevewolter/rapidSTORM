#ifndef SIMPARM_TREEENTRY_HH
#define SIMPARM_TREEENTRY_HH

#include "simparm/Object.h"
#include "simparm/Attribute.h"

namespace simparm {

class TreeObject : public Object {
  protected:
    NodeHandle create_hidden_node( simparm::NodeHandle );
    NodeHandle make_naked_node( simparm::NodeHandle );
  public:
    Attribute<bool> show_in_tree, force_new_root,
                    focus_immediately;

    TreeObject( std::string name, std::string desc );
};

}

#endif
