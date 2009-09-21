#ifndef SIMPARM_ENTRY_PROGRESS
#define  SIMPARM_ENTRY_PROGRESS

#include <simparm/NumericEntry.hh>

namespace simparm {

class ProgressEntry : public DoubleEntry {
  protected:
    virtual string getTypeDescriptor() const
        { return "ProgressEntry"; }
    class ASCII_Progress_Shower;
    std::auto_ptr<ASCII_Progress_Shower> display;

  public:
    virtual ~ProgressEntry();
    ProgressEntry(const ProgressEntry &entry);
    ProgressEntry(string name, string desc,
                        double value = 0, double increment = 0.01);
    virtual ProgressEntry* clone() const
        { return new ProgressEntry(*this); }

    void makeASCIIBar(std::ostream &where);
    void stopASCIIBar();
};

}

#endif
