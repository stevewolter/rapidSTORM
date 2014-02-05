#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "input_simulation/NoiseSource.h"
#include "core/Config.h"

using namespace dStorm::output;

namespace input_simulation {

void input_simulation ( dStorm::Config& config ) {
    config.add_input( new NoiseConfig(), dStorm::InputMethod );
}

}
