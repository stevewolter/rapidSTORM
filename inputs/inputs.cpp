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

namespace dStorm {

using engine::StormPixel;

void basic_inputs( input::Config* inputConfig ) {
    inputConfig->file_method.add_choice( 
        new LocalizationFile::Reader::ChainLink() );
#ifdef HAVE_LIBREADSIF
    inputConfig->file_method.add_choice( 
        new input::AndorSIF::Config<StormPixel>() );
#endif
#ifdef HAVE_TIFFIO_H
    inputConfig->file_method.add_choice( 
        new TIFF::ChainLink<StormPixel>() );
#endif
#if defined(HAVE_LIBATMCD32D) || defined(HAVE_LIBDUMMYANDORCAMERA)
    inputConfig->method.add_choice( new AndorCamera::Method() );
#endif
    
}

}
