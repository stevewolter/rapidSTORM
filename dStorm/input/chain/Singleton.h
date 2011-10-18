#ifndef DSTORM_INPUT_CHAINSINGLETON_H
#define DSTORM_INPUT_CHAINSINGLETON_H

#include "../../helpers/thread.h"
#include "Forwarder.h"
#include <boost/utility.hpp>
#include <set>
#include <stdexcept>

namespace dStorm {
namespace input {
namespace chain {

class Singleton : public Forwarder, boost::noncopyable {
    typedef std::set<Link*> Upstream;
    Upstream less_specializeds;
    ost::Mutex mutex;

    typedef Link::SetType SetType;
    virtual void set_upstream_element( Link& element, SetType type );

  public:
    Singleton();
    ~Singleton();
    Singleton* clone() const 
        { throw std::logic_error("dStorm::input::chain::Singleton is SINGLETON and not cloneable"); }

    void set_more_specialized(Link* link) 
        { Forwarder::set_more_specialized_link_element(link); }
    void remove_less_specialized(Link* link);

    BaseSource* makeSource();
    AtEnd traits_changed( TraitsRef r, Link* which );
    AtEnd context_changed( ContextRef, Link* );
    simparm::Node& getNode();
};

}
}
}

#endif
