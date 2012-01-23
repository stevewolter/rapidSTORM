#include "fwd.h"
#include <dStorm/Config.h>

namespace dStorm {
namespace viewer {

void augment_config( dStorm::Config& config ) {
    config.add_output( make_output_source() );
}

}
}
