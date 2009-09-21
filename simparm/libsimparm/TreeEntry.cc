#include "Attribute.hh"
#include "TreeEntry.hh"

namespace simparm {

_TreeAttributes::_TreeAttributes()
        : show_in_tree("show_in_tree", true),
          force_new_root("force_new_root", false),
          focus_immediately("focus_immediately", false) 
{
}

_TreeAttributes::~_TreeAttributes() {
    removeFromAllParents();
}

}
