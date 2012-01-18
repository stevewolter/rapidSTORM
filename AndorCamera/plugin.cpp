#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <dStorm/Config.h>
#include <dStorm/input/Link.h>

namespace dStorm {
namespace AndorCamera {

std::auto_ptr< dStorm::input::Link > get_method();

void augment_config ( dStorm::Config& config ) {
    config.add_input( dStorm::AndorCamera::get_method(), dStorm::InputMethod );
}

}
}
