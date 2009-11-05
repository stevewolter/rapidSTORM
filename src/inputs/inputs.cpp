#include "config.h"

#include <dStorm/input/Config.h>
#include <CImg.h>
#include <dStorm/output/LocalizationFileReader.h>
#include "AndorSIF.h"
#include "TIFF.h"
#ifdef HAVE_LIBATMCD32D
#include "AndorCamera/AndorDirect.h"
#endif

#include <simparm/ChoiceEntry_Impl.hh>

namespace dStorm {

void basic_inputs( CImgBuffer::Config* inputConfig ) {
    inputConfig->inputMethod.addChoice( 
        new LocalizationFileReader::Config( *inputConfig ) );
#ifdef HAVE_LIBREADSIF
    inputConfig->inputMethod.addChoice( 
        new CImgBuffer::AndorSIF::Config<StormPixel>( *inputConfig ) );
#endif
    inputConfig->inputMethod.addChoice( 
        new CImgBuffer::TIFF::Config<StormPixel>( *inputConfig ) );
#ifdef HAVE_LIBATMCD32D
    inputConfig->inputMethod.addChoice( 
        new dStorm::AndorDirect::Config( *inputConfig ) );
#endif
    
}

}
