#ifndef DSTORM_INPUT_ALTERNATIVES_H
#define DSTORM_INPUT_ALTERNATIVES_H

#include "Link.h"
#include "Filter.h"
#include <simparm/ChoiceEntry.hh>
#include <set>

namespace dStorm {
namespace input {
namespace chain {

class Alternatives
: public Link, public simparm::NodeChoiceEntry<chain::Filter>, 
  public simparm::Listener
{
    class UpstreamCollector;
    std::auto_ptr<UpstreamCollector> collector;

    ContextRef current_context;
    boost::shared_ptr<Context> no_throw_context;

    virtual void downstream_element_destroyed( Link& which );

  protected:
    virtual void operator()(const simparm::Event&);
  public:
    typedef simparm::NodeChoiceEntry<Filter> ChoiceEntry;

    Alternatives(std::string name, std::string desc);
    Alternatives(const Alternatives&);
    ~Alternatives();

    void set_more_specialized_link_element( Link* );
    
    virtual AtEnd traits_changed( TraitsRef, Link* );
    virtual AtEnd context_changed( ContextRef, Link* );

    virtual BaseSource* makeSource();
    virtual Alternatives* clone() const;
    virtual simparm::Node& getNode();

    virtual void add_choice( Filter&, ChoiceEntry::iterator where );
    virtual void remove_choice( Filter& );

    void push_back_choice( Filter& l);
    void throw_exception_for_invalid_configuration() const;
    void insert_new_node( std::auto_ptr<Link>, Place );

    operator const simparm::Node&() const { return *this; }
    operator simparm::Node&() { return *this; }
};

}
}
}

#endif
