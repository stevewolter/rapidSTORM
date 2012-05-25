#include "ProgressEntry.h"
#include "Node.h"
#include <math.h>

namespace simparm {

NodeHandle ProgressEntry::create_hidden_node( NodeHandle at ) {
    NodeHandle r = Entry<double>::create_hidden_node( at );
    r->add_attribute( indeterminate );
    return r;
}

NodeHandle ProgressEntry::make_naked_node( NodeHandle node ) {
    return node->create_progress_bar( getName() );
}

ProgressEntry::ProgressEntry(const ProgressEntry &entry)
: Entry<double>(entry), indeterminate(entry.indeterminate)
{
    this->increment = 0.01;
}

ProgressEntry::ProgressEntry(string name, string desc, double value)
: Entry<double>(name, desc, value), indeterminate("indeterminate", false)
{
   min = (0);
   max = (1+1E-10);
   increment = 0.01;
}

ProgressEntry::~ProgressEntry() {}

}
