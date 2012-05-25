#ifndef SIMPARM_BASIC_ENTRY_HH
#define SIMPARM_BASIC_ENTRY_HH

#include "iostream.h"
#include "BoostOptional.h"
#include "Object.h"
#include "Attribute.h"
#include <boost/utility.hpp>
#include <boost/utility/base_from_member.hpp>

namespace simparm {

class BasicEntry : public Object {
  public:
    Attribute<std::string> help;
    Attribute<bool> invalid, editable;
private:
    Attribute<bool> outputOnChange;
public:
    Attribute<std::string> helpID;

protected:
    NodeHandle create_hidden_node( NodeHandle );
    NodeHandle create_textfield( NodeHandle parent, std::string name, std::string type );
    NodeHandle create_checkbox( NodeHandle parent, std::string name );
    NodeHandle create_choice( NodeHandle parent, std::string name );
public:
    BasicEntry(std::string name, std::string desc = "");
    BasicEntry(const BasicEntry&);
    ~BasicEntry() ;

    void setHelp(const std::string &help)
        { this->help = help; }
    void setInvalid(const bool &invalid)
        { this->invalid = invalid; }
    void setEditable(const bool &editable)
        { this->editable = editable; }
};


}

#endif
