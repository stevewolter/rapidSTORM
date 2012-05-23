#ifndef SIMPARM_NODE
#define SIMPARM_NODE

#if 0
#include <list>
#include <stdexcept>
#include <memory>

#include "Link.hh"
#include "Link_iterator.hh"

#include "Callback.hh"

#include <iostream>
#endif

#include <string>
#include <memory>
#include "Message.hh"
#include "BaseAttribute.hh"

namespace simparm {

struct Node {
    virtual ~Node() {}
    virtual std::auto_ptr<Node> create_object( std::string name ) = 0;
    virtual std::auto_ptr<Node> create_entry( std::string name, std::string desc, std::string type ) = 0;
    virtual std::auto_ptr<Node> create_set( std::string name ) = 0;
    virtual std::auto_ptr<Node> create_choice( std::string name, std::string desc ) = 0;
    virtual std::auto_ptr<Node> create_file_entry( std::string name, std::string desc ) = 0;
    virtual std::auto_ptr<Node> create_progress_bar( std::string name, std::string desc ) = 0;
    virtual std::auto_ptr<Node> create_trigger( std::string name, std::string desc ) = 0;
    virtual void add_attribute( simparm::BaseAttribute& ) = 0;
    virtual Message::Response send( Message& m ) = 0;
    virtual void show() = 0;
    /** TODO: Method is deprecated and should be removed on successful migration. */
    virtual bool isActive() const = 0;
};

typedef Node& NodeRef;

}

#endif
