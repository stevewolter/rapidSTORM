#ifndef SIMPARM_OBJECTCHOICE_HH
#define SIMPARM_OBJECTCHOICE_HH

#include <simparm/Object.hh>

namespace simparm {

class ObjectChoice {
protected:
    simparm::Object node;
public:
    ObjectChoice( std::string name, std::string desc )
        : node(name,desc) {}
    virtual ~ObjectChoice() {}
    virtual ObjectChoice* clone() const = 0;
    std::string getName() const { return node.getName(); }
    simparm::Node& getNode() { return node; }
    const simparm::Node& getNode() const { return node; }
    void attach_ui( simparm::Node& to ) { node.attach_ui(to); }
    void detach_ui( simparm::Node& to ) { node.detach_ui(to); }
};

}

#endif
