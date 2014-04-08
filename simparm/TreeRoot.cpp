#include "simparm/TreeRoot.h"
#include "simparm/Node.h"

namespace simparm {

simparm::NodeHandle TreeRoot::attach_ui( simparm::NodeHandle h ) {
    simparm::NodeHandle rv( h->create_tree_root() );
    rv->initialization_finished();
    return rv;
}

}
