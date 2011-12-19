#include "../debug.h"

#include "Alternatives.h"
#include "Forwarder.h"
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Message.hh>
#include <map>
#include "Forwarder.h"
#include "../InputMutex.h"
#include <dStorm/input/chain/MetaInfo.h>

namespace dStorm {
namespace input {
namespace chain {

class Alternatives::UpstreamCollector 
: public Forwarder, boost::noncopyable
{
    Alternatives& papa;

  public:
    UpstreamCollector(Alternatives& papa) : papa(papa) {}
    UpstreamCollector(Alternatives& papa, const UpstreamCollector& o)
        : papa(papa) {}

    UpstreamCollector* clone() const 
        { assert(false); throw std::logic_error("UpstreamCollector unclonable"); }

    BaseSource* makeSource() { return Forwarder::makeSource(); }

    AtEnd traits_changed( TraitsRef r, Link* from ) {
        DEBUG("Traits changed from upstream in " << this  << " from " << from << " to " << r.get() );
        assert( upstream_traits() == r );
        Link::traits_changed(r,from);
        AtEnd rv;
        for ( Alternatives::iterator i = papa.beginChoices(); i != papa.endChoices(); ++i ) {
            rv = i->traits_changed( r, this );
            if ( upstream_traits() != r ) return AtEnd();
        }
        return rv;
    }
};

Alternatives::Alternatives(std::string name, std::string desc, bool auto_select)
: Choice(name, desc, auto_select), collector( new UpstreamCollector(*this) )
{
}

Alternatives::Alternatives(const Alternatives& o)
: Choice(o), collector( new UpstreamCollector(*this, *o.collector) )
{
    for ( Alternatives::iterator i = beginChoices(); i != endChoices(); ++i ) {
        dynamic_cast<Forwarder&>(*i).more_specialized = collector.get();
    }
    receive_changes_from( value );
}

Alternatives::~Alternatives() {
    stop_receiving_changes_from(value);
}

void Alternatives::set_more_specialized_link_element( Link* l ) {
    collector->set_more_specialized_link_element( l );
}
    
void Alternatives::add_choice( std::auto_ptr<Link> choice ) 
{
    dynamic_cast<Forwarder&>(*choice).more_specialized = collector.get();
    Choice::add_choice(choice);
}

void Alternatives::insert_new_node( std::auto_ptr<Link> link, Place p )
{
    collector->insert_new_node(link,p);
}

}
}
}
