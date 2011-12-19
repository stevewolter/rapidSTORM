#include "debug.h"

#include "Config.h"
#include <simparm/ChoiceEntry_Iterator.hh>
#include <algorithm>
#include <functional>
#include "chain/Link.h"
#include "chain/Context.h"
#include "join.h"
#include "InputMethods.h"
#include <dStorm/input/chain/Forwarder.h>

namespace dStorm {
namespace input {

struct Config::InputChainBase 
: public chain::Forwarder
{
    InputChainBase* clone() const { return new InputChainBase(); }
    AtEnd traits_changed( TraitsRef t, Link* l ) 
        { chain::Link::traits_changed(t,l);
          return notify_of_trait_change(t); }
    AtEnd context_changed( ContextRef t, Link* l ) 
        { chain::Link::context_changed(t,l);
          return notify_of_context_change(t); }
    BaseSource* makeSource() { return Forwarder::makeSource(); }
};

Config::Config()
: simparm::Set("Input", "Input options"),
  method( join::create_link( std::auto_ptr<chain::Link>( new InputMethods() ) ) ),
  terminal_node( new InputChainBase() )
{
    push_back( method->getNode() );
    terminal_node->set_more_specialized_link_element( method.get() );
}

Config::Config(const Config& c)
: simparm::Set(c),
  method( c.method->clone() ),
  terminal_node( new InputChainBase(*c.terminal_node) )
{
    push_back( method->getNode() );
    terminal_node->set_more_specialized_link_element( method.get() );

    for ( boost::ptr_list<chain::Link>::const_iterator 
          i = c.forwards.begin(); i != c.forwards.end(); ++i )
    {
        add_filter( std::auto_ptr<chain::Link>( i->clone() ) );
    }
}

Config::~Config() {
    terminal_node->set_more_specialized_link_element( NULL );
    dynamic_cast< dStorm::input::chain::Forwarder& >(forwards.back()).set_more_specialized_link_element( NULL );
}

void Config::add_method( std::auto_ptr<chain::Link> method, chain::Link::Place p )
{
    this->method->insert_new_node( method, p );
}

void Config::add_filter( std::auto_ptr<chain::Link> forwarder, bool front )
{
    if ( front && ! forwards.empty() ) {
        dynamic_cast< chain::Forwarder& >(forwards.front()).set_more_specialized_link_element(NULL);
    } else {
        terminal_node->set_more_specialized_link_element(NULL);
    }
    if ( forwards.empty() || front ) {
        dynamic_cast< chain::Forwarder& >(*forwarder).set_more_specialized_link_element( method.get() );
    } else {
        dynamic_cast< chain::Forwarder& >(*forwarder).set_more_specialized_link_element( &forwards.back() );
    }
    if ( front && ! forwards.empty() ) {
        dynamic_cast< chain::Forwarder& >(forwards.front()).set_more_specialized_link_element( forwarder.get() );
    } else
        terminal_node->set_more_specialized_link_element( forwarder.get() );


    if ( front ) {
        simparm::Node::iterator i = this->begin();
        while ( &*i != &forwards.front().getNode() ) ++i;
        insert( i, forwarder->getNode() );
        forwards.push_front( forwarder );
    } else {
        push_back( forwarder->getNode() );
        forwards.push_back( forwarder );
    }
}

#if 0
simparm::Attribute<std::string>& Config::input_file() {
    return file_method->input_file.value;
}
#endif

chain::Link& Config::get_link_element() {
    return *terminal_node;
}
const chain::Link& Config::get_link_element() const {
    return *terminal_node;
}

}
}
