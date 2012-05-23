#include "Object.hh"
#include <algorithm>
#include <numeric>
#include <functional>
#include <cassert>

#include <map>

using namespace std;

namespace simparm {

#ifndef NDEBUG
extern void announce_dead_node( Node *node, std::string );
#endif

Object::Object(string name, string desc)
: desc("desc", desc),
  viewable("viewable", true),
  userLevel("userLevel", Beginner),
  name(name)
{
}

Object::Object(const Object& o)
: desc(o.desc), viewable(o.viewable),
  userLevel(o.userLevel), name(o.name)
{}

Object::~Object() { 
}

NodeHandle Object::attach_ui( simparm::NodeHandle node ) {
    if ( node ) {
        NodeHandle r = create_hidden_node( node );
        r->show();
        return r;
    } else {
        return node;
    }
}

void Object::detach_ui( simparm::NodeHandle node ) {
    node_.reset();
}

NodeHandle Object::create_hidden_node( simparm::NodeHandle node ) { 
    node_ = make_naked_node( node );
    node_->add_attribute(desc);
    node_->add_attribute(viewable);
    node_->add_attribute(userLevel);
    return node_;
}

NodeHandle Object::make_naked_node( simparm::NodeHandle node ) {
    return node->create_object( name );
}

std::string Object::getName() const { return name; }
std::string Object::getDesc() const { return desc(); }

};
