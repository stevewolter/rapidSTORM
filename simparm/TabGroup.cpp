#include "simparm/TabGroup.h"
#include "simparm/Node.h"

namespace simparm {

TabGroup::TabGroup(std::string name, std::string desc)
: Object(name, desc)
{
}

NodeHandle TabGroup::make_naked_node( simparm::NodeHandle node ) {
    return node->create_tab_group( getName() );
}

}
