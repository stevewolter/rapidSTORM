#ifndef SIMPARM_NUMERIC_ENTRY
#define SIMPARM_NUMERIC_ENTRY

#include "BoostOptional.h"
#include "Entry.h"
#include "UnitDeclarator.h"
#include "MinMaxWatcher.h"
#include "default_value.h"

namespace simparm {

#if 0
template <typename NumType>
class Entry : public ValueEntry<NumType> {
    typedef typename add_boost_optional<NumType>::type bound_type;
  public:
    typedef NumType value_type;

    Attribute<NumType> increment;
    Attribute< bound_type > min, max;

    void setMin(NumType new_min) { min = new_min; }
    void setMax(NumType new_max) { max = new_max; }
    void setNoMin() { min = bound_type(); }
    void setNoMax() { max = bound_type(); }
    void setIncrement(NumType new_increment) 
        { increment = new_increment; }

  private:
    UnitDeclarator<NumType> unit;
    BoundWatcher<bound_type,NumType,true> value_above_min;
    BoundWatcher<NumType,bound_type,true> max_above_value;
    BoundWatcher<bound_type,NumType,false> value_below_max;
    BoundWatcher<NumType,bound_type,false> min_below_value;

    void registerNamedEntries();
};
#endif

}

#endif
