#ifndef SIMPARM_NUMERIC_ENTRY_IMPL_HH
#define SIMPARM_NUMERIC_ENTRY_IMPL_HH

#include "Entry.hh"
#include "Entry_Impl.hh"
#include "Set.hh"
#include "IO.hh"
#include <stdlib.h>
#include <sstream>
#include <limits>
#include <cmath>

namespace simparm {

template <typename TypeOfEntry>
Entry<TypeOfEntry>::Entry
    (const Entry<TypeOfEntry> &entry)
: ValueEntry<TypeOfEntry>(entry),
  increment(entry.increment),
  min(entry.min),
  max(entry.max),
  value_above_min( min, NULL ),
  max_above_value( this->value, NULL ),
  value_below_max( max, &value_above_min ),
  min_below_value( this->value, NULL )
{
    registerNamedEntries();
}

template <typename TypeOfEntry>
void Entry<TypeOfEntry>::registerNamedEntries() {
    this->value.change_is_OK = &value_below_max;
    min.change_is_OK = &min_below_value;
    max.change_is_OK = &max_above_value;

    this->push_back(increment);
    this->push_back(min);
    this->push_back(max);
    unit.registerNamedEntries( *this );
}

template <typename TypeOfEntry>
Entry<TypeOfEntry>::Entry
    (string name, string desc, TypeOfEntry val, TypeOfEntry inc)
: ValueEntry<TypeOfEntry>(name, desc, val),
    increment("increment", inc),
    min("min", bound_type() ),
    max("max", bound_type() ),
    value_above_min( min, NULL ),
    max_above_value( this->value, NULL ),
    value_below_max( max, &value_above_min ),
    min_below_value( this->value, NULL )
{
    registerNamedEntries();
}

template <typename TypeOfEntry>
Entry<TypeOfEntry>::~Entry()
{
}

}

#endif
