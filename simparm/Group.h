#ifndef SIMPARM_GROUP_H
#define SIMPARM_GROUP_H

#include "Object.h"

namespace simparm {
class Group : public Object {
  protected:
    NodeHandle make_naked_node( simparm::NodeHandle node );
  public:
    Group(std::string name, std::string desc);
    Group(std::string name);
    Group* clone() const { return new Group(*this); }
};
}

#endif

