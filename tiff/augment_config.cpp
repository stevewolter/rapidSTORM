#include <dStorm/Config.h>
#include "RawImageFile.h"
#include "TIFF.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace dStorm { 
    class Config;

namespace tiff { 

void augment_config ( dStorm::Config& config ) {
    config.add_output( new output::RawImageFile::Source() );
#ifdef HAVE_TIFFIO_H
    config.add_input( make_input(), dStorm::FileReader );
#endif

}

}
}

