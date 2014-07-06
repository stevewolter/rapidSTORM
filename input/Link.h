#ifndef DSTORM_INPUT_MANAGER_H
#define DSTORM_INPUT_MANAGER_H

#include "input/fwd.h"

#include <boost/shared_ptr.hpp>
#include <string>
#include <list>
#include <utility>
#include <memory>
#include "simparm/NodeHandle.h"
#include "InsertionPlace.h"
#include <boost/ptr_container/clone_allocator.hpp>
#include <boost/signals2/slot.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/signals2/signal_type.hpp>
#include <boost/signals2/dummy_mutex.hpp>

namespace dStorm {
namespace input {

class Link {
  protected:
    typedef boost::shared_ptr<const MetaInfo> TraitsRef;
  private:
    typedef boost::signals2::keywords::mutex_type<boost::signals2::dummy_mutex>
        no_mutex;
    typedef boost::signals2::signal_type< void (TraitsRef), no_mutex >::type 
        TraitsSignal;
  public:
    typedef std::auto_ptr<boost::signals2::scoped_connection> Connection;
  private:
    TraitsRef meta_info;
    TraitsSignal meta_info_signal;

    virtual BaseSource* makeSource() = 0;

  public:
    Link();
    Link(const Link&);
    virtual ~Link();

    std::auto_ptr<BaseSource> make_source();
    virtual Link* clone() const = 0;
    virtual void registerNamedEntries( simparm::NodeHandle ) = 0;

    virtual void publish_meta_info() = 0;
    TraitsRef current_meta_info() const { return meta_info; }

    typedef dStorm::InsertionPlace Place;
    virtual void insert_new_node( std::auto_ptr<Link>, Place ) = 0;

    Connection notify( const TraitsSignal::slot_type& whom );
 
  protected:
    virtual void update_current_meta_info( TraitsRef new_traits );
};

/** Specialized case of Link for terminal nodes, that is,
 *  those that need no further elements up the chain. */
struct Terminus : public Link {
    virtual Terminus* clone() const = 0;
    virtual void insert_new_node( std::auto_ptr<Link> l, Place ); 
};

}
}

namespace boost {

template <>
inline dStorm::input::Link* new_clone<dStorm::input::Link>( const dStorm::input::Link& l )
    { return l.clone(); }
template <>
inline void delete_clone<dStorm::input::Link>(const dStorm::input::Link* l) 
    { delete l; }

}


#endif
