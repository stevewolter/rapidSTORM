#include "TriggerEntry.hh"
#include <stdlib.h>

namespace simparm {

TriggerEntry::TriggerEntry(const TriggerEntry &entry)
: Entry<unsigned long>(entry) 
{
}

TriggerEntry::TriggerEntry(string name, string desc)
: Entry<unsigned long>(name, desc, 0) 
{
}

std::auto_ptr<Node> TriggerEntry::make_naked_node( simparm::Node& node ) {
    return node.create_trigger( getName(), getDesc() );
}


}
