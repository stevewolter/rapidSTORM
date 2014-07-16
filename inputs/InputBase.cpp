#include "inputs/InputBase.h"

#include "input/Forwarder.h"
#include "helpers/make_unique.hpp"
#include "simparm/Group.h"

namespace dStorm {

struct InputChainBase 
: public input::Forwarder
{
    simparm::Group input_config;
    InputChainBase() : input_config("Input") {}

    InputChainBase* clone() const OVERRIDE { return new InputChainBase(*this); }
    std::string name() const OVERRIDE { return input_config.getName(); }
    void registerNamedEntries( simparm::NodeHandle node ) OVERRIDE {
        Forwarder::registerNamedEntries( input_config.attach_ui(node) );
    }
};

std::unique_ptr< input::Link > make_input_base() {
    return make_unique<InputChainBase>();
}

}
