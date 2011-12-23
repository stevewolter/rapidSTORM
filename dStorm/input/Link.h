#ifndef DSTORM_INPUT_MANAGER_H
#define DSTORM_INPUT_MANAGER_H

#include "fwd.h"

#include <boost/shared_ptr.hpp>
#include <string>
#include <list>
#include <utility>
#include <memory>
#include <simparm/Node_decl.hh>
#include <dStorm/InsertionPlace.h>
#include <boost/ptr_container/clone_allocator.hpp>

namespace dStorm {
namespace input {

class Link {
  protected:
    typedef boost::shared_ptr<const MetaInfo> TraitsRef;
  private:
    Link *less_specialized;
    TraitsRef meta_info;
  public:
    Link();
    Link(const Link&);
    virtual ~Link();

    /** Method which is called by the downstream element (e.g. input) 
     *  to notify the upstream element (e.g. engine) of changed capabilities. */
    virtual void traits_changed( TraitsRef, Link* ) = 0;

    virtual BaseSource* makeSource() = 0;
    virtual Link* clone() const = 0;
    virtual void registerNamedEntries( simparm::Node& ) = 0;

    virtual void publish_meta_info() = 0;
    TraitsRef current_meta_info() const { return meta_info; }

    typedef dStorm::InsertionPlace Place;
    virtual void insert_new_node( std::auto_ptr<Link>, Place ) = 0;
    virtual std::string name() const = 0;
    virtual std::string description() const = 0;
 
  protected:
    enum SetType { Add, Remove };
    virtual void set_upstream_element( Link& element, SetType );
    static void set_upstream_element( Link& on, Link& to, SetType type )
        { on.set_upstream_element(to, type); }
    void update_current_meta_info( TraitsRef new_traits );
};

/** Specialized case of Link for terminal nodes, that is,
 *  those that need no further elements up the chain. */
struct Terminus : public Link {
    /** This method makes no sense for a terminus, since it should be
     *  called by predecessors, which are nonexistent. It is only defined
     *  for its virtual table entry and will simply throw an error when
     *  called. */
    virtual void traits_changed( TraitsRef, Link* );

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
