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
    push_back(this->desc);
    push_back(this->viewable);
    push_back(this->userLevel);

}
Object::Object(const Object& o)
: Node(o), desc(o.desc),
    viewable(o.viewable),
    userLevel(o.userLevel)
{
    push_back(desc);
    push_back(viewable);
    push_back(userLevel);
}
Object::~Object() { 
    clearParents(); 
}

void Object::attach_ui( simparm::Node& node ) {
    node.push_back( *this );
}

void Object::detach_ui( simparm::Node& node ) {
    node.erase( *this );
}

};
