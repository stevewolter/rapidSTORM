#ifndef SIMPARM_OBJECT_HH
#define SIMPARM_OBJECT_HH

#include <string>
#include "Node.hh"
#include "Attribute.hh"
#include <memory>

namespace simparm {

class Object {
  public:
    enum UserLevel { Beginner = 10, Intermediate = 20, 
                     Expert = 30, Debug = 40 };
    friend std::istream& operator>>(std::istream &i, UserLevel& ul)
        { int v; i >> v; ul = (UserLevel)v; return i; }

  public:
    Object(std::string name, std::string desc);
    Object( const Object& );
    ~Object();

    Attribute<std::string> desc;
    Attribute<bool> viewable;
    Attribute<UserLevel> userLevel;

    std::string getTypeDescriptor() const  
        { return "Object"; }
    std::string getName() const;
    std::string getDesc() const;

    void setDesc(const std::string& new_desc);
    void setViewable(const bool &viewable)
        { this->viewable = viewable; }
    void setUserLevel(UserLevel level)
        { this->userLevel = level; }
    NodeHandle attach_ui( simparm::NodeHandle node );
    void detach_ui( simparm::NodeHandle node );

    NodeHandle get_user_interface_handle() { return node_; }

protected:
    virtual NodeHandle create_hidden_node( simparm::NodeHandle );
    virtual NodeHandle make_naked_node( simparm::NodeHandle );
private:
    NodeHandle node_;
    std::string name;
};

}

#endif
