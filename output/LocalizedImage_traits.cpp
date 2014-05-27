#include "output/LocalizedImage_traits.h"

namespace dStorm {
namespace input {

Traits<output::LocalizedImage>::Traits( const std::string& name, const std::string& description )
: carburettor(NULL), engine(NULL), 
  name(name), description(description) {}

Traits<output::LocalizedImage>::Traits(
    const Traits<Localization>& traits,
    const std::string& name,
    const std::string& desc,
    engine::Input* carburettor,
    Engine *repeater)
: Traits<Localization>(traits),
    carburettor(carburettor),
    engine(repeater),
    name(name), description(desc)
    {}

Traits<output::LocalizedImage>::~Traits() {}

}

}
