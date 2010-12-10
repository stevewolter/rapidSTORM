#ifndef SIMPARM_ConfigEntryTypeImpl
#define SIMPARM_ConfigEntryTypeImpl

namespace simparm {

template <typename EntryType>
ConfigEntryType<EntryType>::ConfigEntryType(
    string name, string desc, const EntryType& startVal) throw()
: ConfigObject(name,desc),
  ConfigEntry(name,desc), 
  value("value", startVal)
{
    push_back(value);
}

template <typename EntryType>
ConfigEntryType<EntryType>::ConfigEntryType(
    const ConfigEntryType<EntryType>& from) throw()
: ConfigObject(from),
  ConfigEntry(from), 
  value(from.value)
{
    push_back(value);
}

template <typename EntryType>
ConfigEntryType<EntryType>::~ConfigEntryType() throw() 
{
}

template <typename EntryType>
ConfigEntryType<EntryType> &ConfigEntryType<EntryType>::operator=
    (const ConfigEntryType<EntryType> &entry) throw()
{
        desc = entry.desc;
        help = entry.help;
        invalid = entry.invalid;
        viewable = entry.viewable;
        editable = entry.editable;
	outputOnChange = entry.outputOnChange;
        value = entry.value;
	return *this;
}

template <typename EntryType>
istream& ConfigEntryType<EntryType>::readValue(istream &i) 
    { return i >> value; }


}

#endif
