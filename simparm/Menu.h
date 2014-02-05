#ifndef SIMPARM_MENU
#define SIMPARM_MENU

#include "simparm/Object.h"

namespace simparm {

class Menu : public Object {
    virtual string getTypeDescriptor() const
        { return string("Menu"); }
  public:
    Menu(std::string name, std::string desc) : Object(name, desc) {}
    Menu *clone() const { return new Menu(*this); }
};

}

#endif
