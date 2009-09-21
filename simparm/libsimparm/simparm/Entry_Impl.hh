#ifndef SIMPARM_EntryTypeImpl
#define SIMPARM_EntryTypeImpl

#include <simparm/Entry.hh>

namespace simparm {

template <typename TypeOfEntry>
EntryType<TypeOfEntry>::EntryType(
    string name, string desc, const TypeOfEntry& startVal)
: Entry(name,desc), 
  value("value", startVal)
{
    push_back(value);
}

template <typename TypeOfEntry>
EntryType<TypeOfEntry>::EntryType(
    const EntryType<TypeOfEntry>& from)
: Node(from),
  Entry(from), 
  value(from.value)
{
    push_back(value);
}

template <typename TypeOfEntry>
EntryType<TypeOfEntry>::~EntryType() 
{
}

template <typename TypeOfEntry>
EntryType<TypeOfEntry> &EntryType<TypeOfEntry>::operator=
    (const EntryType<TypeOfEntry> &entry)
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

}

#endif
