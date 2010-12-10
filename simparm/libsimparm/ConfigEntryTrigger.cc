#include "ConfigEntry.hh"
#include <stdlib.h>

namespace simparm {

ConfigEntryTrigger::ConfigEntryTrigger(const ConfigEntryTrigger &entry) throw()
: ConfigObject(entry), ConfigEntryType<unsigned long>(entry) 
{
}

ConfigEntryTrigger::ConfigEntryTrigger(string name, string desc) throw()
: ConfigObject(name, desc), ConfigEntryType<unsigned long>(name, desc, 0) 
{
}

std::istream& ConfigEntryTrigger::readValue(istream& i) {
    std::string value;
    i >> value;
    if (value == "trigger")
        trigger();
    else {
        int j;
        std::stringstream(value) >> j;
        this->value = j;
    }
    return i;
}

}
