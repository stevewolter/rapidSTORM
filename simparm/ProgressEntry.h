#ifndef SIMPARM_ENTRY_PROGRESS
#define  SIMPARM_ENTRY_PROGRESS

#include "Entry.h"

namespace simparm {

class ProgressEntry : public Entry<double> {
  protected:
    virtual string getTypeDescriptor() const
        { return "ProgressEntry"; }
    NodeHandle create_hidden_node( NodeHandle );
    NodeHandle make_naked_node( NodeHandle );

  public:
    virtual ~ProgressEntry();
    ProgressEntry(const ProgressEntry &entry);
    ProgressEntry(string name, string desc, double value = 0);
    virtual ProgressEntry* clone() const
        { return new ProgressEntry(*this); }

    Attribute<bool> indeterminate;
};

}

#endif
