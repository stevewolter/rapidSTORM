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

}
