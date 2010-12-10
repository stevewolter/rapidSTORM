#include "ConfigEntry.hh"
#include "ConfigSet.hh"
#include "ConfigIO.hh"
#include <stdlib.h>
#include <sstream>
#include <limits>

namespace simparm {

template <typename EntryType>
class ConfigEntryNumeric<EntryType>::MinMaxWatcher
: public ConfigNode::Callback {
  private:
    ConfigAttribute<bool> &hasMin, &hasMax;
    ConfigAttribute<EntryType> &value, &min, &max;

  public:
    MinMaxWatcher( ConfigAttribute<bool>& hasMin,
                   ConfigAttribute<bool>& hasMax,
                   ConfigAttribute<EntryType>& value,
                   ConfigAttribute<EntryType>& min,
                   ConfigAttribute<EntryType>& max ) throw()
    : hasMin(hasMin), hasMax(hasMax), value(value), min(min), max(max)
    {
        hasMin.addChangeCallback(*this);
        hasMax.addChangeCallback(*this);
        min.addChangeCallback(*this);
        max.addChangeCallback(*this);
        value.addChangeCallback(*this);
        (*this)( value, ValueChanged, NULL );
    }

    ~MinMaxWatcher() throw() {
        hasMin.removeChangeCallback(*this);
        hasMax.removeChangeCallback(*this);
        min.removeChangeCallback(*this);
        max.removeChangeCallback(*this);
        value.removeChangeCallback(*this);
    }
    void operator()(ConfigNode&, Cause cause, ConfigNode *) throw() {
        if ( cause == ValueChanged ) {
            bool changed = false;
            EntryType curVal = value();
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

template <typename EntryType>
class ConfigEntryNumeric<EntryType>::IncrementWatcher 
: public ConfigAttribute<EntryType>::ChangeWatchFunction
{
    ConfigAttribute<EntryType>& increment;
  public:
    IncrementWatcher(ConfigAttribute<EntryType>& increment) : increment(increment) {}
    bool operator()(const EntryType& a, const EntryType &b)
        { return ( ((a-b) > 0 ? a-b : b-a) > increment); }
};

template <typename EntryType>
ConfigEntryNumeric<EntryType>::ConfigEntryNumeric
    (const ConfigEntryNumeric<EntryType> &entry) throw()
: ConfigObject(entry),
  ConfigEntryType<EntryType>(entry),
  hasMin(entry.hasMin),
  hasMax(entry.hasMax),
  increment(entry.increment),
  min(entry.min),
  max(entry.max)
{
    watcher.reset( 
        new MinMaxWatcher( hasMin, hasMax, this->value, min, max ) );
    inc_watcher.reset( new IncrementWatcher( increment ) );
}

template <typename EntryType>
ConfigEntryNumeric<EntryType>::ConfigEntryNumeric
    (string name, string desc, EntryType val, EntryType inc) throw()
: ConfigObject(name, desc),
  ConfigEntryType<EntryType>(name, desc, val),
    hasMin("has_min", false),
    hasMax("has_max", false),
    increment("increment", inc),
    min("min", std::numeric_limits<EntryType>::min() ),
    max("max", std::numeric_limits<EntryType>::max() )
{
    watcher.reset( 
        new MinMaxWatcher( hasMin, hasMax, this->value, min, max ) );
    inc_watcher.reset( new IncrementWatcher( increment ) );
}

template <typename EntryType>
ConfigEntryNumeric<EntryType>::~ConfigEntryNumeric() throw()
{
    watcher.reset(NULL);
    inc_watcher.reset(NULL);
}

}
