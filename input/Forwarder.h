#ifndef DSTORM_INPUT_FORWARDER_H
#define DSTORM_INPUT_FORWARDER_H

#include "input/Link.hpp"

namespace dStorm {
namespace input {

template <typename InputType, typename OutputType = InputType>
class Forwarder : public Link<OutputType> {
    typedef typename Link<OutputType>::Connection Connection;
    typedef typename Link<OutputType>::TraitsRef TraitsRef;

    std::unique_ptr<Link<InputType>> more_specialized;
    Connection connection;

  public:
    Forwarder(std::unique_ptr<Link<InputType>> upstream)
        : more_specialized(std::move(upstream)) {}

    Forwarder(const Forwarder& o) {
        if ( o.more_specialized.get() )
            more_specialized.reset( o.more_specialized->clone() );
        if ( more_specialized.get() )
            connection = more_specialized->notify( 
                boost::bind( &Forwarder::traits_changed, this, _1, more_specialized.get() ) );
    }

    Forwarder* clone() const = 0;
    Source<OutputType>* makeSource() = 0;

    void registerNamedEntries( simparm::NodeHandle h ) OVERRIDE {
        more_specialized->registerNamedEntries(h);
    }

    std::string name() const OVERRIDE {
        return more_specialized->name();
    }

    void publish_meta_info() OVERRIDE {
        more_specialized->publish_meta_info();
    }

    virtual void traits_changed( TraitsRef ref, Link<InputType>* ) {
        this->update_current_meta_info(ref);
    }

  protected:
    TraitsRef upstream_traits() const {
        return more_specialized->current_meta_info();
    }
    std::unique_ptr< Source<InputType> > upstream_source() {
        return more_specialized->make_source();
    }
    Link<InputType>* get_more_specialized() {
        return more_specialized.get();
    }
};

}
}

#endif
