#ifndef SIMPARM_NODE
#define SIMPARM_NODE

#include <string>
#include "Message.h"
#include "BaseAttribute.h"
#include "NodeHandle.h"

namespace simparm {

struct Node {
    virtual ~Node() {}
    virtual NodeHandle create_object( std::string name ) = 0;
    virtual NodeHandle create_entry( std::string name, std::string desc, std::string type ) = 0;
    virtual NodeHandle create_set( std::string name ) = 0;
    virtual NodeHandle create_choice( std::string name, std::string desc ) = 0;
    virtual NodeHandle create_file_entry( std::string name, std::string desc ) = 0;
    virtual NodeHandle create_progress_bar( std::string name, std::string desc ) = 0;
    virtual NodeHandle create_trigger( std::string name, std::string desc ) = 0;
    virtual void add_attribute( simparm::BaseAttribute& ) = 0;
    virtual Message::Response send( Message& m ) const = 0;
    virtual void show() = 0;
    /** TODO: Method is deprecated and should be removed on successful migration. */
    virtual bool isActive() const = 0;
};

}

#endif
