#ifndef SIMPARM_OBJECT_HH
#define SIMPARM_OBJECT_HH

#include <string>
#include <simparm/Node.hh>
#include <simparm/Attribute.hh>

namespace simparm {
using std::string;

class Object : public virtual Node {
  public:
    enum UserLevel { Beginner = 10, Intermediate = 20, 
                     Expert = 30, Debug = 40 };
    friend std::istream& operator>>(std::istream &i, UserLevel& ul)
        { int v; i >> v; ul = (UserLevel)v; return i; }

  public:
    Object(string name, string desc);
    Object(const Object& o);
    virtual ~Object();

    Attribute<std::string> name;
    Attribute<std::string> desc;
    Attribute<bool> viewable;
    Attribute<UserLevel> userLevel;

    std::string getTypeDescriptor() const  
        { return "Object"; }
    std::string getName() const { return name(); }
    std::string getDesc() const { return desc(); }

    void setName(const std::string& to) { name = to; }
    void setDesc(const std::string& new_desc) { desc = new_desc; }
    void setViewable(const bool &viewable)
        { this->viewable = viewable; }
    void setUserLevel(UserLevel level)
        { this->userLevel = level; }

    virtual Object *clone() const { return new Object(*this); }
  private:
    class NameWatcher;
    std::auto_ptr< NameWatcher > watcher;
};

}

#endif
