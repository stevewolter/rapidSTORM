#ifndef SIMPARM_NODE
#define SIMPARM_NODE

#include <string>
#include "Message.h"
#include "BaseAttribute.h"
#include "NodeHandle.h"
#include "UserLevel.h"

namespace simparm {

struct Node {
    virtual ~Node() {}
    virtual NodeHandle create_object( std::string name ) = 0;
    virtual NodeHandle create_textfield( std::string name, std::string type ) = 0;
    virtual NodeHandle create_checkbox( std::string name ) = 0;
    virtual NodeHandle create_group( std::string name ) = 0;
    virtual NodeHandle create_tab_group( std::string name ) = 0;
    virtual NodeHandle create_choice( std::string name ) = 0;
    virtual NodeHandle create_file_entry( std::string name ) = 0;
    virtual NodeHandle create_progress_bar( std::string name ) = 0;
    virtual NodeHandle create_trigger( std::string name ) = 0;
    virtual void add_attribute( simparm::BaseAttribute& ) = 0;
    virtual Message::Response send( Message& m ) const = 0;
    virtual void initialization_finished() = 0;
    /** TODO: Method is deprecated and should be removed on successful migration. */
    virtual bool isActive() const = 0;
    virtual void set_description( std::string ) = 0;
    virtual void set_visibility( bool ) = 0;
    virtual void set_user_level( UserLevel arg ) = 0;
    virtual void set_help_id( std::string ) = 0;
    virtual void set_help( std::string ) = 0;
    virtual void set_editability( bool ) = 0;
};

}

#endif
