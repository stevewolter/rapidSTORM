#ifndef SIMPARM_EntryTypeImpl
#define SIMPARM_EntryTypeImpl

#include "Entry.hh"

namespace simparm {

template <typename TypeOfEntry>
Entry<TypeOfEntry>::Entry(
    string name, string desc, const TypeOfEntry& startVal)
: BasicEntry(name,desc), 
  boost::base_from_member< Attribute<TypeOfEntry> >("value", startVal),
  Attributes<TypeOfEntry>(boost::base_from_member< Attribute<TypeOfEntry> >::member),
  value( boost::base_from_member< Attribute<TypeOfEntry> >::member )
{
    Attributes<TypeOfEntry>::registerNamedEntries(*this);
    push_back(value);
}

template <typename TypeOfEntry>
Entry<TypeOfEntry>::Entry(
    const Entry<TypeOfEntry>& from)
: BasicEntry(from), 
  boost::base_from_member< Attribute<TypeOfEntry> >(from.member),
  Attributes<TypeOfEntry>(from, boost::base_from_member< Attribute<TypeOfEntry> >::member),
  value( boost::base_from_member< Attribute<TypeOfEntry> >::member )
{
    Attributes<TypeOfEntry>::registerNamedEntries(*this);
    push_back(value);
}

template <typename TypeOfEntry>
Entry<TypeOfEntry> &
Entry<TypeOfEntry>::operator=(const Entry<TypeOfEntry> &entry)
{
    this->member = entry.member;
    static_cast<Attributes<TypeOfEntry>&>(*this) = entry;
    return *this;
}

template <typename TypeOfEntry>
Entry<TypeOfEntry>::~Entry() {}

template <typename TypeOfEntry, typename ValueField>
Attributes<TypeOfEntry, ValueField, 
    typename boost::enable_if< boost::is_fundamental<TypeOfEntry> >::type
>::Attributes(const Attributes &entry, Attribute<ValueField>& value )
: increment(entry.increment),
  min(entry.min),
  max(entry.max),
  value_above_min( min, NULL ),
  max_above_value( value, NULL ),
  value_below_max( max, &value_above_min ),
  min_below_value( value, NULL )
{
    value.change_is_OK = &value_below_max;
    min.change_is_OK = &min_below_value;
    max.change_is_OK = &max_above_value;
}

template <typename TypeOfEntry, typename ValueField>
void Attributes<TypeOfEntry, ValueField, 
    typename boost::enable_if< boost::is_fundamental<TypeOfEntry> >::type 
>::registerNamedEntries( simparm::Node& n ) {
    n.push_back(increment);
    n.push_back(min);
    n.push_back(max);
}

template <typename TypeOfEntry, typename ValueField>
Attributes<TypeOfEntry, ValueField, 
    typename boost::enable_if< boost::is_fundamental<TypeOfEntry> >::type 
>::Attributes(Attribute<ValueField>& value)
: 
    increment("increment", default_increment(ValueField())),
    min("min", bound_type() ),
    max("max", bound_type() ),
    value_above_min( min, NULL ),
    max_above_value( value, NULL ),
    value_below_max( max, &value_above_min ),
    min_below_value( value, NULL )
{
    value.change_is_OK = &value_below_max;
    min.change_is_OK = &min_below_value;
    max.change_is_OK = &max_above_value;
}

}

#endif
