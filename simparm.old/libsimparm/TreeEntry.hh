#ifndef SIMPARM_TREEENTRY_HH
#define SIMPARM_TREEENTRY_HH

#include "Node.hh"
#include "Attribute.hh"
#include "Structure.hh"

namespace simparm {

class TreeAttributes {
  protected:
  public:
    Attribute<bool> show_in_tree, force_new_root,
                    focus_immediately;

    TreeAttributes();

    void registerNamedEntries(Node& node) {
      node.push_back( show_in_tree );
      node.push_back( force_new_root); 
      node.push_back( focus_immediately );
    }
};

}

#endif
