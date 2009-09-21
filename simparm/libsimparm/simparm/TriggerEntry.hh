#ifndef SIMPARM_TRIGGER_ENTRY
#define SIMPARM_TRIGGER_ENTRY

#include <simparm/Entry.hh>

namespace simparm {

class TriggerEntry : public EntryType<unsigned long> {
  protected:
    virtual string getTypeDescriptor() const
        { return string("TriggerEntry"); }

  public:
    TriggerEntry(const TriggerEntry &entry);
    TriggerEntry(string name, string desc);
    virtual ~TriggerEntry() {}
    virtual TriggerEntry* clone() const
        { return new TriggerEntry(*this); }

    unsigned long pending() const { return value; }
    bool triggered() const { return value; }

    void trigger() { value += 1; }
    void untrigger() { value -= 1; }
    void setTrigger(bool trigger) 
        { if (trigger) this->trigger(); else untrigger(); }

    TriggerEntry &operator=(bool triggered)
        { setTrigger(triggered); return *this; }

    bool operator==(const bool &triggered) const
        { return this->triggered() == triggered; }
    bool operator!=(const bool &triggered) const
        { return this->triggered() != triggered; }
};

}

#endif
