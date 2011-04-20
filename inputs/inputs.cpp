#include "config.h"

#include <dStorm/engine/Image_decl.h>
#include <dStorm/input/Config.h>
#include <dStorm/localization_file/reader.h>
#include "AndorSIF.h"
#include "TIFF.h"
#include "BackgroundDeviationEstimator_decl.h"
#include "Splitter_decl.h"
#include "YMirror_decl.h"
#include "SampleInfo.h"
#include "ResolutionSetter_decl.h"

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

    inputConfig->add_filter( Splitter::makeLink(), true );
    inputConfig->add_filter( YMirror::makeLink() );
    inputConfig->add_filter( BackgroundStddevEstimator::makeLink() );
    inputConfig->add_filter( input::sample_info::makeLink() );
    std::cerr << "Adding resolution filter" << std::endl;
    inputConfig->add_filter( input::Resolution::makeLink() );
    
}

}
