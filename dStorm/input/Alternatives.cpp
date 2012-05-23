#include "debug.h"

#include "Alternatives.h"
#include "Forwarder.h"
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Message.hh>
#include <map>
#include "Forwarder.h"
#include "InputMutex.h"
#include <dStorm/input/MetaInfo.h>

namespace dStorm {
namespace input {

class Alternatives::UpstreamCollector 
: public Forwarder
{
  public:
    UpstreamCollector* clone() const { return new UpstreamCollector(*this); }
    std::string name() const { return "AlternativesUpstreamCollector"; }
    std::string description() const { throw std::logic_error("Not implemented"); }
};

class Alternatives::UpstreamLink
: public Link 
{
    UpstreamCollector& collector;
    Link::Connection connection;
    void update_current_meta_info( TraitsRef t ) { 
        Link::update_current_meta_info(t); 
    }

  public:
    UpstreamLink( UpstreamCollector& collector ) 
        : collector(collector), connection( collector.notify( 
            boost::bind( &UpstreamLink::update_current_meta_info, this, _1 ) ) ) {}
    /** This clone() implementation returns no object. The new upstream links
     *  are inserted by the Alternatives class. */
    UpstreamLink* clone() const { return NULL; }
    void registerNamedEntries( simparm::NodeHandle ) {}
    std::string name() const { return "AlternativesUpstreamLink"; }
    std::string description() const { throw std::logic_error("Not implemented"); }
    BaseSource* makeSource() { return collector.makeSource(); }
    void insert_new_node( std::auto_ptr<Link>, Place ) { throw std::logic_error("Not implemented"); }
    void publish_meta_info() {}
};

Alternatives::Alternatives(std::string name, std::string desc, bool auto_select)
: Choice(name, desc, auto_select), collector( new UpstreamCollector() )
{
}

Alternatives::Alternatives(const Alternatives& o)
: Choice(o), collector( o.collector->clone() )
{
    for ( Choice::iterator i = choices.begin(); i != choices.end(); ++i ) {
        i->link().insert_new_node( std::auto_ptr< Link >(new UpstreamLink(*collector) ), Anywhere );
    }
}

Alternatives::~Alternatives() {
}

void Alternatives::add_choice( std::auto_ptr<Link> choice ) 
{
    choice->insert_new_node( std::auto_ptr< Link >(new UpstreamLink(*collector) ), Anywhere );
    Choice::add_choice(choice);
}

void Alternatives::insert_new_node( std::auto_ptr<Link> link, Place p )
{
    collector->insert_new_node(link,p);
}

void Alternatives::registerNamedEntries( simparm::NodeHandle node ) {
    collector->registerNamedEntries(node);
    Choice::registerNamedEntries(node);
}

void Alternatives::publish_meta_info() {
    collector->publish_meta_info();
    Choice::publish_meta_info();
    assert( current_meta_info().get() );
}

}
}
