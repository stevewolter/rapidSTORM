#include "LocalizedImage_traits.h"

namespace dStorm {
namespace input {

Traits<output::LocalizedImage>::Traits(
    const Traits<Localization>& traits,
    /*const std::string& name,
    const std::string& desc,*/
    engine::Input* carburettor,
    Engine *repeater)
: Traits<Localization>(traits),
    carburettor(carburettor),
    engine(repeater),
    source_image_is_set(false),
    smoothed_image_is_set(false),
    candidate_tree_is_set(false)/*,
    name(name), description(desc)*/
    {}

Traits<output::LocalizedImage>::~Traits() {}

}

}
