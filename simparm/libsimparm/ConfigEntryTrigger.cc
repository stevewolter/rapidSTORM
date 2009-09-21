#include "TriggerEntry.hh"
#include <stdlib.h>

namespace simparm {

TriggerEntry::TriggerEntry(const TriggerEntry &entry)
: Node(entry), EntryType<unsigned long>(entry) 
{
}

TriggerEntry::TriggerEntry(string name, string desc)
: Node(), EntryType<unsigned long>(name, desc, 0) 
{
}

}
