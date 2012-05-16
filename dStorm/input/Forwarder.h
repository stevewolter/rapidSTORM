#ifndef DSTORM_INPUT_FORWARDER_H
#define DSTORM_INPUT_FORWARDER_H

#include "Link.h"

namespace dStorm {
namespace input {

class Forwarder : public Link {
    std::auto_ptr<Link> more_specialized;
    Connection connection;
  public:
    Forwarder();
    Forwarder(const Forwarder&);
    ~Forwarder();

    virtual Forwarder* clone() const = 0;
    virtual BaseSource* makeSource();
    void insert_new_node( std::auto_ptr<Link>, Place );
    void registerNamedEntries( simparm::Node& );
    std::string name() const;
    void publish_meta_info();

    virtual void traits_changed( TraitsRef, Link* );

  protected:
    void insert_here( std::auto_ptr<Link> );
    TraitsRef upstream_traits() const;
    std::auto_ptr< BaseSource > upstream_source();
};

}
}

#endif
