#ifndef DSTORM_INPUT_FORWARDER_H
#define DSTORM_INPUT_FORWARDER_H

#include "input/Link.h"

namespace dStorm {
namespace input {

class Forwarder : public Link {
    std::unique_ptr<Link> more_specialized;
    Connection connection;
  public:
    Forwarder();
    Forwarder(const Forwarder&);
    ~Forwarder();

    virtual Forwarder* clone() const = 0;
    virtual BaseSource* makeSource();
    void insert_new_node( std::unique_ptr<Link> );
    void registerNamedEntries( simparm::NodeHandle );
    std::string name() const;
    void publish_meta_info();

    virtual void traits_changed( TraitsRef, Link* );

  protected:
    void insert_here( std::unique_ptr<Link> );
    TraitsRef upstream_traits() const;
    std::unique_ptr< BaseSource > upstream_source();
    Link* get_more_specialized() {
        return more_specialized.get();
    }
};

}
}

#endif
