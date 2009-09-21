#ifndef SIMPARM_TREEENTRY_HH
#define SIMPARM_TREEENTRY_HH

#include <simparm/Node.hh>
#include <simparm/Structure.hh>

namespace simparm {

class _TreeAttributes : public virtual simparm::Node {
  protected:
    void registerNamedEntries() {
      push_back( show_in_tree );
      push_back( force_new_root); 
      push_back( focus_immediately );
    }

  public:
    Attribute<bool> show_in_tree, force_new_root,
                    focus_immediately;

    _TreeAttributes();

    virtual ~_TreeAttributes();
};

typedef VirtualStructure<_TreeAttributes> TreeAttributes;

}

#endif
