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

class Object::NameWatcher 
: public Attribute<std::string>::ChangeWatchFunction
{
  public:
    bool operator()(const std::string&, const std::string& to)

    {
        if ( to == "" ) return false;
        for (unsigned int i = 0; i < to.size(); i++)
            if ( !isalnum( to[i] ) && to[i] != '_' )
                return false;
        return true;
    }
};

Object::Object(string name, string desc)
: Node(), name("name", name), desc("desc", desc),
  viewable("viewable", true),
  userLevel("userLevel", Beginner)
{
    watcher.reset( new NameWatcher() );
    assert( /* Name has 1-n characters in [A-Za-z_] */ 
            (*watcher)("",name) );
    this->name.change_is_OK = ( watcher.get() );
    push_back(this->name);
    push_back(this->desc);
    push_back(this->viewable);
    push_back(this->userLevel);

}
Object::Object(const Object& o)
: Node(o), name(o.name), desc(o.desc),
    viewable(o.viewable),
    userLevel(o.userLevel)
{
    watcher.reset( new NameWatcher() );
    name.change_is_OK = ( watcher.get() );
    push_back(name);
    push_back(desc);
    push_back(viewable);
    push_back(userLevel);
}
Object::~Object() { 
    removeFromAllParents(); 
}

};
