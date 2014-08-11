#include "inputs/InputBase.h"

#include "input/Forwarder.h"
#include "helpers/make_unique.hpp"
#include "simparm/Group.h"

namespace dStorm {

template <typename Type>
struct InputChainBase 
: public input::Forwarder<Type>
{
    simparm::Group input_config;
    InputChainBase(std::unique_ptr<input::Link<Type>> upstream)
        : input::Forwarder<Type>(std::move(upstream)), input_config("Input") {}

    InputChainBase* clone() const OVERRIDE { return new InputChainBase(*this); }
    std::string name() const OVERRIDE { return input_config.getName(); }
    void registerNamedEntries( simparm::NodeHandle node ) OVERRIDE {
        input::Forwarder<Type>::registerNamedEntries( input_config.attach_ui(node) );
    }
    input::Source<Type>* makeSource() OVERRIDE { return this->upstream_source().release(); }
};

std::unique_ptr< input::Link<engine::ImageStack> > make_image_input_base(
    std::unique_ptr<input::Link<engine::ImageStack>> upstream) {
    return make_unique<InputChainBase<engine::ImageStack>>(std::move(upstream));
}

std::unique_ptr< input::Link<output::LocalizedImage> > make_localization_input_base(
    std::unique_ptr<input::Link<output::LocalizedImage>> upstream) {
    return make_unique<InputChainBase<output::LocalizedImage>>(std::move(upstream));
}

}
