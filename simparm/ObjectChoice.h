#ifndef SIMPARM_OBJECTCHOICE_HH
#define SIMPARM_OBJECTCHOICE_HH

#include "simparm/Object.h"
#include "simparm/Choice.h"

namespace simparm {

class ObjectChoice : public Choice {
    simparm::Object node;
protected:
    NodeHandle attach_parent( simparm::NodeHandle to ) { return node.attach_ui(to); }
    void set_viewability( bool v ) { this->node.set_visibility(v); }
public:
    ObjectChoice( std::string name, std::string desc )
        : node(name,desc) {}
    ObjectChoice( std::string name ) : node(name) {}
    virtual ~ObjectChoice() {}
    virtual ObjectChoice* clone() const = 0;
    std::string getName() const { return node.getName(); }
    virtual void attach_ui( simparm::NodeHandle to ) = 0;
};

}

#endif
