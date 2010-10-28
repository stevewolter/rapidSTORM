#include "config.h"

#include <dStorm/input/Config.h>
#include <dStorm/localization_file/reader.h>
#include "AndorSIF.h"
#include "TIFF.h"
#if defined(HAVE_LIBATMCD32D) || defined(HAVE_LIBDUMMYANDORCAMERA)
#include "AndorCamera/InputChainLink.h"
#endif

#include <simparm/ChoiceEntry_Impl.hh>
#include <dStorm/input/FileMethod.h>
#include "BackgroundDeviationEstimator_decl.h"

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
#if defined(HAVE_LIBATMCD32D) || defined(HAVE_LIBDUMMYANDORCAMERA)
    inputConfig->add_method( new AndorCamera::Method() );
#endif

    inputConfig->add_filter( BackgroundStddevEstimator::makeLink() );
    
}

}
