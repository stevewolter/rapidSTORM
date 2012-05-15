#ifndef SIMPARM_OBJECTCHOICE_HH
#define SIMPARM_OBJECTCHOICE_HH

#include <simparm/Object.hh>

namespace simparm {

class ObjectChoice {
    simparm::Object node;
protected:
    NodeRef attach_parent( simparm::Node& to ) { return node.attach_ui(to); }
    void set_viewability( bool v ) { this->node.viewable = v; }
public:
    ObjectChoice( std::string name, std::string desc )
        : node(name,desc) {}
    virtual ~ObjectChoice() {}
    virtual ObjectChoice* clone() const = 0;
    std::string getName() const { return node.getName(); }
    simparm::Node& getNode() { return node; }
    const simparm::Node& getNode() const { return node; }
    virtual void attach_ui( simparm::Node& to ) = 0;
    void detach_ui( simparm::Node& to ) { node.detach_ui(to); }
};

}

#endif
