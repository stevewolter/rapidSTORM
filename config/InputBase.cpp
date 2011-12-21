#include "InputBase.h"
#include <simparm/Set.hh>
#include <dStorm/input/chain/Forwarder.h>

namespace dStorm {

struct InputChainBase 
: public input::chain::Forwarder
{
    simparm::Set input_config;
    InputChainBase() : input_config("Input", "Input options") {}

    InputChainBase* clone() const { return new InputChainBase(); }
    std::string name() const { return input_config.getName(); }
    std::string description() const { return input_config.getDesc(); }
    void registerNamedEntries( simparm::Node& node ) {
        Forwarder::registerNamedEntries( input_config );
        node.push_back( input_config );
    }
    void insert_new_node( std::auto_ptr<Link> l, Place p ) {
        if ( p == BeforeEngine ) 
            Forwarder::insert_here( l );
        else
            Forwarder::insert_new_node( l, p );
    }
};

std::auto_ptr< input::chain::Link > make_input_base() {
    return std::auto_ptr< input::chain::Link >( new InputChainBase() );
}

}
