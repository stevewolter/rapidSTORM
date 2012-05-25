#include "InputBase.h"
#include <simparm/Group.h>
#include <dStorm/input/Forwarder.h>

namespace dStorm {

struct InputChainBase 
: public input::Forwarder
{
    simparm::Group input_config;
    InputChainBase() : input_config("Input", "Input options") {}
    ~InputChainBase() {}

    InputChainBase* clone() const { return new InputChainBase(*this); }
    std::string name() const { return input_config.getName(); }
    std::string description() const { return input_config.getDesc(); }
    void registerNamedEntries( simparm::NodeHandle node ) {
        Forwarder::registerNamedEntries( input_config.attach_ui(node) );
    }
    void insert_new_node( std::auto_ptr<Link> l, Place p ) {
        if ( p == BeforeEngine ) 
            Forwarder::insert_here( l );
        else
            Forwarder::insert_new_node( l, p );
    }
};

std::auto_ptr< input::Link > make_input_base() {
    return std::auto_ptr< input::Link >( new InputChainBase() );
}

}
