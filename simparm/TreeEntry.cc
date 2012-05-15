#include "Attribute.hh"
#include "TreeEntry.hh"

namespace simparm {

TreeAttributes::TreeAttributes()
        : show_in_tree("show_in_tree", true),
          force_new_root("force_new_root", false),
          focus_immediately("focus_immediately", false) 
{
}

}
