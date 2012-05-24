#ifndef SIMPARM_INCREMENTWATCHER_HH
#define SIMPARM_INCREMENTWATCHER_HH

#include "Attribute.h"
#include <cmath>

namespace simparm {

template <typename TypeOfEntry>
class IncrementWatcher
: public Attribute<TypeOfEntry>::ChangeWatchFunction
{
    Attribute<TypeOfEntry>& increment;
  public:
    IncrementWatcher(Attribute<TypeOfEntry>& increment) : increment(increment) {}
    bool operator()(const TypeOfEntry& a, const TypeOfEntry &b) {
        using std::abs;
        TypeOfEntry diff = a-b;
        bool can_return = isnan(a) || isnan(b) || abs( diff ) >= increment();
        return can_return;
    }
};

}

#endif
