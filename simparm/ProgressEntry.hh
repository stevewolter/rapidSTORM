#ifndef SIMPARM_ENTRY_PROGRESS
#define  SIMPARM_ENTRY_PROGRESS

#include "Entry.hh"

namespace simparm {

class ProgressEntry : public Entry<double> {
  protected:
    virtual string getTypeDescriptor() const
        { return "ProgressEntry"; }
    class ASCII_Progress_Shower;
    std::auto_ptr<ASCII_Progress_Shower> display;

    NodeRef create_hidden_node( Node& );
    std::auto_ptr<Node> make_naked_node( simparm::Node& );

  public:
    virtual ~ProgressEntry();
    ProgressEntry(const ProgressEntry &entry);
    ProgressEntry(string name, string desc, double value = 0);
    virtual ProgressEntry* clone() const
        { return new ProgressEntry(*this); }

    void makeASCIIBar(std::ostream &where);
    void stopASCIIBar();

    Attribute<bool> indeterminate;
};

}

#endif
