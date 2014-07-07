#include "base/Config.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace dStorm { 

class Config;

namespace andor_sif { 

std::auto_ptr< input::Link > make_input();

void augment_config ( dStorm::Config& config ) {
#ifdef HAVE_LIBREADSIF
    config.add_input( make_input(), dStorm::FileReader );
#endif

}

}
}

