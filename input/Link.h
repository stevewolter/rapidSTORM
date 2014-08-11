#ifndef DSTORM_INPUT_MANAGER_H
#define DSTORM_INPUT_MANAGER_H

#include <string>
#include <list>
#include <utility>
#include <memory>

#include <boost/shared_ptr.hpp>
#include <boost/signals2/slot.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/signals2/signal_type.hpp>
#include <boost/signals2/dummy_mutex.hpp>

#include "input/MetaInfo.h"
#include "input/Source.h"
#include "simparm/NodeHandle.h"

namespace dStorm {
namespace input {

template <typename Type>
class Link {
  protected:
    typedef boost::shared_ptr<const MetaInfo> TraitsRef;
  private:
    typedef boost::signals2::keywords::mutex_type<boost::signals2::dummy_mutex>
        no_mutex;
    typedef boost::signals2::signal_type< void (TraitsRef), no_mutex >::type 
        TraitsSignal;
  public:
    typedef std::unique_ptr<boost::signals2::scoped_connection> Connection;
  private:
    TraitsRef meta_info;
    TraitsSignal meta_info_signal;

    virtual Source<Type>* makeSource() = 0;

  public:
    Link() {}
    Link(const Link& o) : meta_info(o.meta_info) {}
    virtual ~Link() {}

    std::unique_ptr<Source<Type>> make_source() {
        return std::unique_ptr<Source<Type>>(makeSource());
    }

    virtual Link* clone() const = 0;
    virtual void registerNamedEntries( simparm::NodeHandle ) = 0;

    virtual void publish_meta_info() = 0;
    TraitsRef current_meta_info() const { return meta_info; }

    virtual std::string name() const = 0;

    inline Connection notify( const TraitsSignal::slot_type& whom );
 
  protected:
    inline void update_current_meta_info( TraitsRef new_traits );
};

/** Specialized case of Link for terminal nodes, that is,
 *  those that need no further elements up the chain. */
template <typename Type>
struct Terminus : public Link<Type> {
    virtual Terminus* clone() const = 0;
    void insert_new_node( std::unique_ptr<Link<Type>> l ) OVERRIDE {
        throw std::logic_error("No insertion point found for " + l->name());
    }
};

}
}

#endif
