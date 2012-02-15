#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "NoiseSource.h"
#include <dStorm/Config.h>

using namespace dStorm::output;

namespace locprec {

void input_simulation ( dStorm::Config& config ) {
    config.add_input( new locprec::NoiseConfig(), dStorm::InputMethod );
}

}
