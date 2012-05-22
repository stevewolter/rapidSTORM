#include "Object.hh"
#include <algorithm>
#include <numeric>
#include <functional>
#include <cassert>

#include <map>
#include "stl_helpers.hh"

using namespace std;

namespace simparm {

#ifndef NDEBUG
extern void announce_dead_node( Node *node, std::string );
#endif

Object::Object(string name, string desc)
: Node(name), desc("desc", desc),
  viewable("viewable", true),
  userLevel("userLevel", Beginner)
{
}
Object::~Object() { 
    clearParents(); 
}

NodeRef Object::attach_ui( simparm::Node& node ) {
    push_back(desc);
    push_back(viewable);
    push_back(userLevel);
    node.push_back( *this );
    return *this;
}

void Object::detach_ui( simparm::Node& node ) {
    node.erase( *this );
}

NodeRef Object::invisible_node() { return *this; }

};
