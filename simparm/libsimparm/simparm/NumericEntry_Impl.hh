#include "NumericEntry.hh"
#include "Set.hh"
#include "IO.hh"
#include <stdlib.h>
#include <sstream>
#include <limits>

namespace simparm {

template <typename TypeOfEntry>
class NumericEntry<TypeOfEntry>::MinMaxWatcher
: public Node::Callback {
  private:
    Attribute<bool> &hasMin, &hasMax;
    Attribute<TypeOfEntry> &value, &min, &max;

  public:
    MinMaxWatcher( Attribute<bool>& hasMin,
                   Attribute<bool>& hasMax,
                   Attribute<TypeOfEntry>& value,
                   Attribute<TypeOfEntry>& min,
                   Attribute<TypeOfEntry>& max )
    : hasMin(hasMin), hasMax(hasMax), value(value), min(min), max(max)
    {
        hasMin.addChangeCallback(*this);
        hasMax.addChangeCallback(*this);
        min.addChangeCallback(*this);
        max.addChangeCallback(*this);
        value.addChangeCallback(*this);
        (*this)( value, ValueChanged, NULL );
    }

    ~MinMaxWatcher() {
        hasMin.removeChangeCallback(*this);
        hasMax.removeChangeCallback(*this);
        min.removeChangeCallback(*this);
        max.removeChangeCallback(*this);
        value.removeChangeCallback(*this);
    }
    void operator()(Node&, Cause cause, Node *) {
        if ( cause == ValueChanged ) {
            bool changed = false;
            TypeOfEntry curVal = value();
            if ( hasMin() && curVal < min() ) {
                changed = true;
                curVal = min();
            } else if ( hasMax() && curVal > max() ) {
                changed = true;
                curVal = max();
            }
            if (changed)
                value = curVal;
        }
    }
};

template <typename TypeOfEntry>
class NumericEntry<TypeOfEntry>::IncrementWatcher 
: public Attribute<TypeOfEntry>::ChangeWatchFunction
{
    Attribute<TypeOfEntry>& increment;
  public:
    IncrementWatcher(Attribute<TypeOfEntry>& increment) : increment(increment) {}
    bool operator()(const TypeOfEntry& a, const TypeOfEntry &b) {
        TypeOfEntry difference = ( (a > b) ? (a-b) : (b-a) );
        bool exceeds = ( difference >= increment() );
        return exceeds; 
    }
};

template <typename TypeOfEntry>
NumericEntry<TypeOfEntry>::NumericEntry
    (const NumericEntry<TypeOfEntry> &entry)
: Node(entry),
  EntryType<TypeOfEntry>(entry),
  hasMin(entry.hasMin),
  hasMax(entry.hasMax),
  increment(entry.increment),
  min(entry.min),
  max(entry.max)
{
    watcher.reset( 
        new MinMaxWatcher( hasMin, hasMax, this->value, min, max ) );
    inc_watcher.reset( new IncrementWatcher( increment ) );

    this->value.change_is_OK = inc_watcher.get();

    this->push_back(increment);
    this->push_back(hasMin);
    this->push_back(min);
    this->push_back(hasMax);
    this->push_back(max);
}

template <typename TypeOfEntry>
NumericEntry<TypeOfEntry>::NumericEntry
    (string name, string desc, TypeOfEntry val, TypeOfEntry inc)
: EntryType<TypeOfEntry>(name, desc, val),
    hasMin("has_min", false),
    hasMax("has_max", false),
    increment("increment", inc),
    min("min", std::numeric_limits<TypeOfEntry>::min() ),
    max("max", std::numeric_limits<TypeOfEntry>::max() )
{
    watcher.reset( 
        new MinMaxWatcher( hasMin, hasMax, this->value, min, max ) );
    inc_watcher.reset( new IncrementWatcher( increment ) );

    this->value.change_is_OK = inc_watcher.get();

    this->push_back(increment);
    this->push_back(hasMin);
    this->push_back(min);
    this->push_back(hasMax);
    this->push_back(max);
}

template <typename TypeOfEntry>
NumericEntry<TypeOfEntry>::~NumericEntry()
{
    watcher.reset(NULL);
    inc_watcher.reset(NULL);
}

}
