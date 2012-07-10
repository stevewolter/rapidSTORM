#include "Group.h"
#include "Node.h"

namespace simparm {

Group::Group(std::string name, std::string desc)
: Object(name, desc)
{
}

Group::Group(std::string name)
: Object(name)
{
}

NodeHandle Group::make_naked_node( simparm::NodeHandle node ) {
    return node->create_group( getName() );
}

}
