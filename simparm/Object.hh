#ifndef SIMPARM_OBJECT_HH
#define SIMPARM_OBJECT_HH

#include <string>
#include "Node.hh"
#include "Attribute.hh"
#include "NodeHandle.hh"

namespace simparm {
using std::string;

class Object : protected Node {
  public:
    enum UserLevel { Beginner = 10, Intermediate = 20, 
                     Expert = 30, Debug = 40 };
    friend std::istream& operator>>(std::istream &i, UserLevel& ul)
        { int v; i >> v; ul = (UserLevel)v; return i; }

  public:
    Object(string name, string desc);
    virtual ~Object();

    Attribute<std::string> desc;
    Attribute<bool> viewable;
    Attribute<UserLevel> userLevel;

    std::string getTypeDescriptor() const  
        { return "Object"; }
    std::string getName() const { return Node::getName(); }
    std::string getDesc() const { return desc(); }

    void setDesc(const std::string& new_desc) { desc = new_desc; }
    void setViewable(const bool &viewable)
        { this->viewable = viewable; }
    void setUserLevel(UserLevel level)
        { this->userLevel = level; }
    NodeRef attach_ui( simparm::Node& node );
    void detach_ui( simparm::Node& node );

    NodeRef invisible_node( simparm::Node& );
    NodeHandle get_user_interface_handle() { return *this; }

    virtual Object *clone() const { return new Object(*this); }
    void clearParents() { Node::clearParents(); }
    void clearChildren() { Node::clearChildren(); }
};

}

#endif
