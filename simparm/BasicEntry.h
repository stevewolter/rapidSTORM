#ifndef SIMPARM_BASIC_ENTRY_HH
#define SIMPARM_BASIC_ENTRY_HH

#include "BoostOptional.h"
#include "Object.h"
#include "Attribute.h"
#include <boost/utility.hpp>
#include <boost/utility/base_from_member.hpp>

namespace simparm {

class BasicEntry : public Object {
    std::string help, helpID;
    bool editable;
protected:
    NodeHandle create_hidden_node( NodeHandle );
    NodeHandle create_textfield( NodeHandle parent, std::string name, std::string type );
    NodeHandle create_checkbox( NodeHandle parent, std::string name );
    NodeHandle create_choice( NodeHandle parent, std::string name );
public:
    BasicEntry(std::string name, std::string desc);
    BasicEntry(std::string name);
    BasicEntry(const BasicEntry&);
    ~BasicEntry() ;

    void setHelp(const std::string &help);
    void setEditable(bool editable);
    void thaw() { setEditable(true); }
    void freeze() { setEditable(false); }
    void setHelpID( const std::string &helpID) ;
};


}

#endif
