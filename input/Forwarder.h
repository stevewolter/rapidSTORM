#ifndef DSTORM_INPUT_FORWARDER_H
#define DSTORM_INPUT_FORWARDER_H

#include "input/Link.h"

namespace dStorm {
namespace input {

template <typename Type>
class Forwarder : public Link<Type> {
    typedef typename Link<Type>::Connection Connection;
    typedef typename Link<Type>::TraitsRef TraitsRef;

    std::unique_ptr<Link<Type>> more_specialized;
    Connection connection;
  public:
    Forwarder();
    Forwarder(const Forwarder&);
    ~Forwarder();

    Forwarder* clone() const = 0;
    Source<Type>* makeSource() OVERRIDE;
    void insert_new_node( std::unique_ptr<Link<Type>> ) OVERRIDE;
    void registerNamedEntries( simparm::NodeHandle ) OVERRIDE;
    std::string name() const OVERRIDE;
    void publish_meta_info() OVERRIDE;

    virtual void traits_changed( TraitsRef, Link<Type>* );

  protected:
    void insert_here( std::unique_ptr<Link<Type>> );
    TraitsRef upstream_traits() const;
    std::unique_ptr< Source<Type> > upstream_source();
    Link<Type>* get_more_specialized() {
        return more_specialized.get();
    }
};

}
}

#endif
