#ifndef DSTORM_INPUT_FORWARDER_H
#define DSTORM_INPUT_FORWARDER_H

#include "Link.h"

namespace dStorm {
namespace input {
namespace chain {

struct Forwarder : public Link {
    Link *more_specialized;
  public:
    Forwarder();
    Forwarder(const Forwarder&);
    Forwarder(Link& more_specialized);
    ~Forwarder();
    void set_more_specialized_link_element(Link* l);

    virtual Forwarder* clone() const = 0;
    virtual BaseSource* makeSource() = 0;
    virtual AtEnd traits_changed( TraitsRef, Link* ) = 0;
    virtual simparm::Node& getNode();
    const simparm::Node& getNode() const { return Link::getNode(); }
    void insert_new_node( std::auto_ptr<Link>, Place );

    AtEnd notify_of_context_change( ContextRef new_context );
    TraitsRef upstream_traits() const;
};

}
}
}

#endif
