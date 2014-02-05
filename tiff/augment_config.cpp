#include <dStorm/Config.h>
#include "tiff/RawImageFile.h"
#include "tiff/TIFF.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace dStorm { 
    class Config;

namespace tiff { 

void input_driver ( dStorm::Config& config ) {
#ifdef HAVE_TIFFIO_H
    config.add_input( make_input(), dStorm::FileReader );
#endif
}

void output_driver ( dStorm::Config& config ) {
#ifdef HAVE_TIFFIO_H
    config.add_output( new output::FileOutputBuilder<output::RawImageFile::Config,output::RawImageFile>() );
#endif
}

}
}

