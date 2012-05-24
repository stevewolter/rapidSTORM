#include "TriggerEntry.h"
#include "Node.h"

namespace simparm {

TriggerEntry::TriggerEntry(const TriggerEntry &entry)
: Entry<unsigned long>(entry) 
{
}

TriggerEntry::TriggerEntry(string name, string desc)
: Entry<unsigned long>(name, desc, 0) 
{
}

NodeHandle TriggerEntry::make_naked_node( simparm::NodeHandle node ) {
    return node->create_trigger( getName(), getDesc() );
}


}
