#ifndef SIMPARM_NUMERIC_ENTRY
#define SIMPARM_NUMERIC_ENTRY

#include <simparm/Entry.hh>

namespace simparm {

template <typename NumType>
class NumericEntry : public EntryType<NumType> {
  private:
    class MinMaxWatcher;
    std::auto_ptr< MinMaxWatcher > watcher;
    class IncrementWatcher;
    std::auto_ptr< IncrementWatcher > inc_watcher;
  public:
    NumericEntry(string name, string desc, 
                        NumType value = 0, NumType increment = 0);
    NumericEntry(const NumericEntry<NumType> &entry);
    ~NumericEntry();
    virtual NumericEntry<NumType>* clone() const
        { return new NumericEntry<NumType>(*this); }

    NumericEntry<NumType> &operator=(const NumericEntry<NumType>& o)
        { this->min = o.min(); this->max = o.max(); this->value = o.value();
          this->increment = o.increment(); return *this; }

    EntryType<NumType> &operator=(NumType entry) 
 { this->value = entry; return *this; }

    Attribute<bool> hasMin, hasMax;
    Attribute<NumType> increment, min, max;

    void setMin(NumType new_min) { min = new_min; hasMin = true; }
    void setMax(NumType new_max) { max = new_max; hasMax = true; }
    void setNoMin() { hasMin = false; }
    void setNoMax() { hasMax = false; }
    void setIncrement(NumType new_increment) 
        { increment = new_increment; }
};

typedef NumericEntry<double> DoubleEntry;
typedef NumericEntry<unsigned long> UnsignedLongEntry;
typedef NumericEntry<long> LongEntry;

}

#endif
