#ifndef SIMPARM_TREEROOT_HH
#define SIMPARM_TREEROOT_HH

#include "simparm/NodeHandle.h"

namespace simparm {

class TreeRoot {
public:
    simparm::NodeHandle attach_ui( simparm::NodeHandle );
};

}

#endif

