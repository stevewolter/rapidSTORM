#ifndef SIMPARM_TABGROUP_H
#define ndef SIMPARM_TABGROUP_H

#include "Object.h"

namespace simparm {
class TabGroup : public Object {
  protected:
    NodeHandle make_naked_node( simparm::NodeHandle node );
  public:
    TabGroup(std::string name, std::string desc);
    TabGroup* clone() const { return new TabGroup(*this); }
};
}

#endif

