#include "inputs/InputBase.h"
#include "simparm/Group.h"
#include "input/Forwarder.h"

namespace dStorm {

struct InputChainBase 
: public input::Forwarder
{
    simparm::Group input_config;
    InputChainBase() : input_config("Input") {}
    ~InputChainBase() {}

    InputChainBase* clone() const { return new InputChainBase(*this); }
    std::string name() const { return input_config.getName(); }
    std::string description() const { return input_config.getDesc(); }
    void registerNamedEntries( simparm::NodeHandle node ) {
        Forwarder::registerNamedEntries( input_config.attach_ui(node) );
    }
};

std::auto_ptr< input::Link > make_input_base() {
    return std::auto_ptr< input::Link >( new InputChainBase() );
}

}
