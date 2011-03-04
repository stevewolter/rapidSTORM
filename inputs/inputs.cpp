#include "config.h"

#include <dStorm/engine/Image_decl.h>
#include <dStorm/input/Config.h>
#include <dStorm/localization_file/reader.h>
#include "AndorSIF.h"
#include "TIFF.h"
#include "AndorCamera/InputChainLink.h"
#include "BackgroundDeviationEstimator_decl.h"
#include "Splitter_decl.h"
#include "YMirror_decl.h"

namespace dStorm {

using engine::StormPixel;

void basic_inputs( input::Config* inputConfig ) {
    inputConfig->add_file_method( 
        new LocalizationFile::Reader::ChainLink() );
#ifdef HAVE_LIBREADSIF
    inputConfig->add_file_method( 
        new input::AndorSIF::Config<StormPixel>() );
#endif
#ifdef HAVE_TIFFIO_H
    inputConfig->add_file_method( new TIFF::ChainLink() );
#endif
    inputConfig->add_method( new AndorCamera::Method() );

    inputConfig->add_filter( Splitter::makeLink(), true );
    inputConfig->add_filter( YMirror::makeLink() );
    inputConfig->add_filter( BackgroundStddevEstimator::makeLink() );
    
}

}
